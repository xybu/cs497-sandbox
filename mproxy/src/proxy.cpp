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
#include <event2/event.h>
#include <event2/util.h>

#define OFP_HEADER_BYTES		(8)
#define OFP_HEADER_LEN_OFFSET	(2)

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

void start_client_thread(Client *client) {
	event_base_dispatch(client->ev_base);
	delete client;
}

/**
 * A wrapper function to call the C version of OFP message handler
 * because loci has error if compiled under C++.
 */
void on_flow_read(struct bufferevent *bev, void *arg) {
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t bytes_to_read = evbuffer_get_length(input);
	size_t len_processed = 0;
	stream_t *msg = ((Client *)arg)->buf;
	stream_t *response;
	int fw_fd = ((Client *)arg)->fw_fd;
	Client *fw_cli = client_map[fw_fd];
	counter_t dup_count;

	dbg(COLOR_YELLOW "on_flow_read: %lu bytes to read on fd %d.\n" COLOR_BLACK, bytes_to_read, ((Client *)arg)->fd);

	if ((msg->cap - msg->len < bytes_to_read) && stream_expand(msg, bytes_to_read)) {
		err(COLOR_RED "on_flow_read: cannot realloc.\n" COLOR_BLACK);
		return;
	}
	
	evbuffer_remove(input, msg->data + msg->len, bytes_to_read);
	msg->len += bytes_to_read;

	// insufficient data
	if (msg->len < OFP_HEADER_BYTES) 
		return;

	//#ifdef _DEBUG
	//stream_dump(msg);
	//#endif

	if (fw_cli == NULL) {
		err(COLOR_RED "on_flow_read: pair has closed. Exit client.\n" COLOR_BLACK);
		event_base_loopexit(((Client *)arg)->ev_base, NULL);
		return;
	}

	while (len_processed + OFP_HEADER_BYTES <= msg->len) {
		unsigned char *ofp_msg = msg->data + len_processed;
		uint16_t ofp_msg_len = *((uint16_t *)(ofp_msg + OFP_HEADER_LEN_OFFSET));
		ofp_msg_len = (ofp_msg_len >> 8) | (ofp_msg_len << 8); // big endian to little endian

		dbg(COLOR_YELLOW "on_flow_read: message length is %u bytes.\n" COLOR_BLACK, ofp_msg_len);
		
		if (ofp_msg_len == 0) {
			// should never be 0
			err(COLOR_RED "on_flow_read: got msg of length 0 on fd %d. Malformed header.\n" COLOR_BLACK, fw_cli->fd);
			event_base_loopexit(((Client *)arg)->ev_base, NULL);
			return;
		}
		if (ofp_msg_len > msg->len - len_processed) {
			if (len_processed > 0) stream_left_shift(msg, len_processed);
			return;
		}
		
		response = action_inject(ofp_msg, ofp_msg_len, &dup_count);
		if (response) {
			if (fw_cli == NULL || !is_valid_fd(fw_fd)) {
				stream_free(response);
				err(COLOR_RED "on_flow_read: Fw fd of client %d has been closed. Stop.\n" COLOR_BLACK, ((Client *)arg)->fd);
				event_base_loopexit(((Client *)arg)->ev_base, NULL);
				return;
			}
			evbuffer_add(fw_cli->ev_buf, response->data, response->len);
			// repeat r times
			while(dup_count > 0) {
				evbuffer_add(fw_cli->ev_buf, response->data, response->len);
				--dup_count;
			}
			stream_free(response);
			if (bufferevent_write_buffer(fw_cli->ev_event, fw_cli->ev_buf)) {
				err(COLOR_RED "on_flow_read: failed to write to fd %d.\n" COLOR_BLACK, fw_cli->fd);
				event_base_loopexit(((Client *)arg)->ev_base, NULL);
				return;
			}
		}
		len_processed += ofp_msg_len;
		dbg(COLOR_GREEN "on_flow_read: processed %lu / %lu bytes.\n" COLOR_BLACK, len_processed, msg->len);
	}
	if (len_processed > 0) stream_left_shift(msg, len_processed);
	dbg(COLOR_GREEN "on_flow_read: finished handling read event on fd %d.\n" COLOR_BLACK, ((Client *)arg)->fd);
}

/**
 * A wrapper function to call the C version of event error handler.
 * read-end if 0x01, write-end if 0x02.
 * EOF = 0x10, unrecoverable rror = 0x20, timeout = 0x40, connect op finished = 0x80.
 */
void on_flow_error(struct bufferevent *bev, short what, void *arg) {
	if (what == '\x11') return; // do not report read EOF
	err(COLOR_RED "on_flow_error: an error occured on fd %d. Err code 0x%x.\n" COLOR_BLACK, ((Client *)arg)->fd, what);
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
	dbg("proxy: sweeping handler threads.\n");
	for (auto it = handler_pool.begin(); it != handler_pool.end(); ++it) {
		(*it)->join();
		delete *it;
	}
	dbg("proxy: freed.\n");
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
	dbg("proxy: stopped with status %d.\n", status);
}

void Proxy::start_listen() {
	SockAddr6 in_addr;
	register socklen_t cli_sock_len = sizeof(SockAddr6);
	register int in_fd, out_fd;
	Client *in_cd, *out_cd;

	dbg("listener: started.\n");
	
	while (is_valid_fd(in_sock_fd)) {
		
		dbg("listener: waiting for requests.\n");

		if ((in_fd = accept(in_sock_fd, (struct sockaddr *)&in_addr, &cli_sock_len)) < 0) {
			if (status != STATUS_STOP) {
				err("listener: got negative in fd. status=%d. in_sock_fd=%d.\n", status, in_sock_fd);
			}
			break;
		}

		dbg(COLOR_CYAN "listener: got request on port %u.\n" COLOR_BLACK, in_addr.sin6_port);

		// set inflow fd nonblock
		if (set_nonblock(in_fd) < 0) {
			err("listener: failed to set fd %d to nonblock mode.\n", in_fd);
			close(in_fd);
			continue;
		}

		// create a new connection to fw host
		if ((out_fd = get_outflow_fd()) < 0) {
			close(in_fd);
			err(COLOR_RED "listener: failed to get a fd to fw host.\n" COLOR_BLACK);
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

		dbg("\033[92mlistener: handled one request (%d, %d).\033[0m\n", in_fd, out_fd);
	}

	dbg("listener: loop broke.\n");
}
