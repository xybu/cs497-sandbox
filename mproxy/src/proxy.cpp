/**
 * proxy.cpp
 * Proxy object accepts incoming requests and create inflow-outflow pairs.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstring>
#include "global.h"
#include "proxy.h"
#include "client_detail.h"
#include "callback.h"

static int set_nonblock(int fd) {
	int flag = fcntl(fd, F_GETFL);
	if (flag < 0) return flag;
	flag |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flag) < 0) return -1;
	return 0;
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
	//for (std::deque<std::thread *>::iterator it = thread_pool.begin(); 
	//		it != thread_pool.end(); ++it) {
	//	*it.terminate();
	//}
	//scheduler.terminate();
	if (in_sock_fd >= 0) close(in_sock_fd);
	if (out_host != NULL) freeaddrinfo(out_host);
	//if (status == STATUS_OK) sem_destroy(&scheduler_sem);
	dprintf("proxy: sweeping handler threads.\n");
	for (auto it = handler_pool.begin(); it != handler_pool.end(); ++it) {
		(*it)->join();
		delete *it;
	}
	dprintf("proxy: freed.\n");
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
	dprintf("proxy: stopped with status %d.\n", status);
}

void Proxy::start_listen() {
	SockAddr6 in_addr;
	register socklen_t cli_sock_len = sizeof(SockAddr6);
	register int in_fd, out_fd;
	ClientDetail *in_cd, *out_cd;

	dprintf("listener: started.\n");
	
	while (is_valid_fd(in_sock_fd)) {
		
		dprintf("listener: waiting for requests.\n");

		if ((in_fd = accept(in_sock_fd, (struct sockaddr *)&in_addr, &cli_sock_len)) < 0) {
			if (status != STATUS_STOP) {
				erprintf("listener: got negative in fd. status=%d. in_sock_fd=%d.\n", status, in_sock_fd);
			}
			break;
		}

		dprintf("\033[94mlistener: got request on port %u.\033[0m\n", in_addr.sin6_port);

		// set inflow fd nonblock
		if (set_nonblock(in_fd) < 0) {
			erprintf("listener: failed to set fd %d to nonblock mode.\n", in_fd);
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
			erprintf("listener: failed to set fd %d to nonblock mode.\n", out_fd);
			close(in_fd);
			close(out_fd);
			continue;
		}

		in_cd = new ClientDetail(in_fd, out_fd);
		if (!in_cd) {
			erprintf("listener: failed to malloc ClientDetail.\n");
			close(in_fd);
			close(out_fd);
			continue;
		}

		if (in_cd->init(callback::on_inflow_read, NULL, callback::on_flow_error)) {
			erprintf("listener: failed to init event.\n");
			close(out_fd);
			delete in_cd;
			continue;
		}

		out_cd = new ClientDetail(out_fd, in_fd);
		if (!out_cd) {
			erprintf("listener: failed to malloc ClientDetail.\n");
			close(out_fd);
			delete in_cd;
			continue;
		}

		if (out_cd->init(callback::on_outflow_read, NULL, callback::on_flow_error)) {
			erprintf("listener: failed to init event.\n");
			delete in_cd;
			delete out_cd;
			continue;
		}

		handler_pool.push_back(new std::thread(&callback::start_event, out_cd));
		handler_pool.push_back(new std::thread(&callback::start_event, in_cd));

		dprintf("\033[92mlistener: handled one request (%d, %d).\033[0m\n", in_fd, out_fd);
	}

	dprintf("listener: loop broke.\n");
}
