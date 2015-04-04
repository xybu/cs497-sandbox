/**
 * proxy.cpp
 * Proxy object accepts incoming requests and create inflow-outflow pairs.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "global.h"
#include "proxy.h"
#include "client.h"
#include "action.h"

/**
 * Set the target file descriptor to non-block mode.
 */
static int set_nonblock(int fd) {
	int flag = fcntl(fd, F_GETFL);
	if (flag < 0) return flag;
	flag |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flag) < 0) return -1;
	return 0;
}

/**
 * A wrapper function to call C function that starts event looping
 * and frees resource when loop ends.
 */
void start_client_thread(Client *client) {
	act_start_loop(client->ev_base);
	delete client;
}

/**
 * A wrapper function to call the C version of event handler
 * because loci has error if compiled under C++.
 */
void on_flow_read(struct bufferevent *bev, void *arg) {
	int ret = act_on_flow_read(((Client *)arg)->ev_base, bev, ((Client *)arg)->ev_buf, ((Client *)arg)->fd, ((Client *)arg)->fw_fd, &(((Client *)arg)->buf));
	if (ret) {
		err(COLOR_RED "on_flow_read: failed to handle event for client %d. Err code %d.\n" COLOR_BLACK, ((Client *)arg)->fd, ret);
	}
}

/**
 * A wrapper function to call the C version of event error handler.
 */
void on_flow_error(struct bufferevent *bev, short what, void *arg) {
	err(COLOR_RED "on_flow_error: an error occured. Err code %d.\n" COLOR_BLACK, what);
}

Proxy::Proxy(int port, char *fw_host, int fw_port) {
	status = STATUS_ERR;
	//client_count = 0;
	in_sock_fd = -1;
	out_host = NULL;
	if (init_in_socket(port) || init_out_socket(fw_host, fw_port)) {
		throw status;
	}
	/*if (sem_init(&scheduler_sem, SEM_PRIVATE, 0) < 0) {
		pperror("sem_init");
		throw status;
	}*/
	status = STATUS_OK;
}

Proxy::~Proxy() {
	//for (std::list<std::thread *>::iterator it = thread_pool.begin(); 
	//		it != thread_pool.end(); ++it) {
	//	*it.terminate();
	//}
	//scheduler.terminate();
	if (in_sock_fd >= 0) close(in_sock_fd);
	if (out_host != NULL) freeaddrinfo(out_host);
	//if (status == STATUS_OK) sem_destroy(&scheduler_sem);
	debug("proxy: sweeping handler threads.\n");
	for (auto it = handler_pool.begin(); it != handler_pool.end(); ++it) {
		(*it)->join();
		delete *it;
	}
	debug("proxy: freed.\n");
}

ProxyStatus Proxy::init_in_socket(int port, int queue_len) {
	int tmp = 0;
	memset(&in_sock, 0, sizeof(SockAddr6));
	if ((in_sock_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		pperror("socket");
		return STATUS_ERR;
	}
	in_sock.sin6_family = AF_INET6;
	in_sock.sin6_addr = in6addr_any;
	in_sock.sin6_port = htons((u_short)port);
	// in_sock.sin6_flowinfo = 0;
	// in_sock.sin6_scope_id = 0;
	if (setsockopt(in_sock_fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int))){
		pperror("setsockopt");
		close(in_sock_fd);
		in_sock_fd = -1;
		return STATUS_ERR;
	}
	if (bind(in_sock_fd, (struct sockaddr*)&in_sock, sizeof(struct sockaddr_in6)) < 0) {
		pperror("bind");
		close(in_sock_fd);
		in_sock_fd = -1;
		return STATUS_ERR;
	}
	if (listen(in_sock_fd, queue_len) < 0) {
		pperror("listen");
		close(in_sock_fd);
		in_sock_fd = -1;
		return STATUS_ERR;
	}
	return STATUS_OK;
}

ProxyStatus Proxy::init_out_socket(char *fw_host, int fw_port) {
	char buf[8];
	struct addrinfo hints;
	snprintf(buf, 6, "%d%c", fw_port, '\0');

	// the outflow socket can reside in both IPv4 and IPv6.
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(fw_host, buf, &hints, &out_host)) {
		pperror("getaddrinfo");
		return STATUS_ERR;
	}
	return STATUS_OK;
}

int Proxy::get_outflow_fd() {
	int fd = socket(out_host->ai_family, out_host->ai_socktype, out_host->ai_protocol);
	if (fd < 0) {
		pperror("socket");
		return STATUS_ERR;
	}
	if (connect(fd, out_host->ai_addr, out_host->ai_addrlen) < 0) {
		pperror("connect");
		close(fd);
		return STATUS_ERR;
	}
	return fd;
}

void Proxy::start() {
	start_listen();
}

void Proxy::stop() {
	status = STATUS_STOP;
	close(in_sock_fd);
	in_sock_fd = -1;
	debug("proxy: stopped with status %d.\n", status);
}

void Proxy::start_listen() {
	SockAddr6 in_addr;
	register socklen_t cli_sock_len = sizeof(SockAddr6);
	register int in_fd, out_fd;
	Client *in_cd, *out_cd;

	debug("listener: started.\n");
	
	while (is_valid_fd(in_sock_fd)) {
		
		debug("listener: waiting for requests.\n");

		if ((in_fd = accept(in_sock_fd, (struct sockaddr *)&in_addr, &cli_sock_len)) < 0) {
			if (status != STATUS_STOP) {
				err("listener: got negative in fd. status=%d. in_sock_fd=%d.\n", status, in_sock_fd);
			}
			break;
		}

		debug(COLOR_CYAN "listener: got request on port %u.\n" COLOR_BLACK, in_addr.sin6_port);

		// set inflow fd nonblock
		if (set_nonblock(in_fd) < 0) {
			err("listener: failed to set fd %d to nonblock mode.\n", in_fd);
			close(in_fd);
			continue;
		}

		// create a new connection to fw host
		if ((out_fd = get_outflow_fd()) < 0) {
			close(in_fd);
			continue;
		}

		// set outflow fd nonblock
		if (set_nonblock(out_fd) < 0) {
			err("listener: failed to set fd %d to nonblock mode.\n", out_fd);
			close(in_fd);
			close(out_fd);
			continue;
		}

		in_cd = new Client(in_fd, out_fd);
		if (!in_cd) {
			err("listener: failed to malloc Client.\n");
			close(in_fd);
			close(out_fd);
			continue;
		}

		if (in_cd->init(on_flow_read, NULL, on_flow_error)) {
			err("listener: failed to init event.\n");
			close(out_fd);
			delete in_cd;
			continue;
		}

		out_cd = new Client(out_fd, in_fd);
		if (!out_cd) {
			err("listener: failed to malloc Client.\n");
			close(out_fd);
			delete in_cd;
			continue;
		}

		if (out_cd->init(on_flow_read, NULL, on_flow_error)) {
			err("listener: failed to init event.\n");
			delete in_cd;
			delete out_cd;
			continue;
		}

		handler_pool.push_back(new std::thread(&start_client_thread, out_cd));
		handler_pool.push_back(new std::thread(&start_client_thread, in_cd));

		debug("\033[92mlistener: handled one request (%d, %d).\033[0m\n", in_fd, out_fd);
	}

	debug("listener: loop broke.\n");
}
