#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include "global.h"
#include "in_socket.h"

// InSocketHandler thread adds entry to fd_map.
// InSocketHandler workers removes entries from fd_map.
std::unordered_map<int, SockResponse *> fd_map;

// OutSocketHandler enqueues fd on done_fds and posts count.
std::deque<int> done_fds;
sem_t done_fds_count;

// a thread-pool based socket server
InSocketHandler::InSocketHandler(int port, int queue_len) {
	int tmp = 0;
	ready = false;
	memset(&in_sock, 0, sizeof(struct sockaddr_in6));
	if ((in_sock_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		throw ready;
	}
	in_sock.sin6_family = AF_INET6;
	in_sock.sin6_flowinfo = 0;
	in_sock.sin6_scope_id = 0;
	in_sock.sin6_addr = in6addr_any;
	in_sock.sin6_port = htons((u_short)port);
	if (setsockopt(in_sock_fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int))){
		perror("setsockopt");
		close(in_sock_fd);
		throw ready;
	}
	if (bind(in_sock_fd, (struct sockaddr*)&in_sock, sizeof(struct sockaddr_in6)) < 0) {
		perror("bind");
		close(in_sock_fd);
		throw ready;
	}
	if (listen(in_sock_fd, queue_len) < 0) {
		perror("listen");
		close(in_sock_fd);
		throw ready;
	}
	if (sem_init(&done_fds_count, SOCKET_SEM_PRIVATE, 0) < 0) {
		perror("sem_init");
		close(in_sock_fd);
		throw ready;
	}
	ready = true;
}

InSocketHandler::~InSocketHandler() {
	if (ready) {
		close(in_sock_fd);
		sem_destroy(&done_fds_count);
	}
}

void InSocketHandler::respond(int id) {
	int fd;
	ssize_t bytes_sent;
	SockResponse *response;
	Stream *msg;

	dprintf("InSocket Worker%d: started.\n", id);
	
	while (1) {
		// get a queued fd
		sem_wait(&done_fds_count);
		fd = done_fds.front();
		done_fds.pop_front();
		response = fd_map[fd];
		dprintf("InSocket Worker%d: got fd %d.\n", id, fd);
		
		// wait for processor thread to finish it
		sem_wait(&response->is_finished);
		msg = response->data;
		if (msg != NULL) {	
			bytes_sent = send(fd, msg->data, msg->len, 0);
			delete msg;
			dprintf("InSocket Worker%d: %lu bytes sent.\n", id, bytes_sent);
		}

		// cleanup
		dprintf("InSocket Worker%d: cleaning up.\n", id);
		close(fd);
		fd_map.erase(fd);
		delete response;
	}
}

void InSocketHandler::start_loop() {
	int i;
	for (i = 0; i < INSOCKET_NUM_WORKERS; ++i) {
		workers[i] = std::thread(&InSocketHandler::respond, this, i);
	}
	while (1) {
		struct sockaddr_in6 cli_sock;
		int cli_sock_fd;
		socklen_t cli_sock_len = sizeof(struct sockaddr_in6);

		dprintf("InSocketHandler: waiting for requests.\n");
		cli_sock_fd = accept(in_sock_fd, (struct sockaddr *)&cli_sock, &cli_sock_len);
		if (cli_sock_fd < 0){
			dprintf("InSocketHandler: negative client socket.\n");
			continue;
		}
		dprintf("InSocketHandler: got one request.\n");

		SockResponse *r = new SockResponse;
		if (sem_init(&r->is_finished, SOCKET_SEM_PRIVATE, 0) < 0) {
			perror("sem_init");
		}
		r->data = NULL;
		queued_fds.push_back(cli_sock_fd);
		fd_map[cli_sock_fd] = r;
		sem_post(&queued_fds_count);
		dprintf("InSocketHandler: queued one request.\n");
	}
}
