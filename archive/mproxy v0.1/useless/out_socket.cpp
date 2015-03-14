#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "global.h"
#include "out_socket.h"

std::deque<int> queued_fds;
sem_t queued_fds_count;

OutSocketHandler::OutSocketHandler(char *host, int port) {
	char buf[8];
	struct addrinfo hints;

	ready = false;
	snprintf(buf, 6, "%d%c", port, '\0');

	// the outflow socket can reside in both IPv4 and IPv6.
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(host, buf, &hints, &out_info)) {
		perror("getaddrinfo");
		throw ready;
	}

/*
	struct hostent *target_host = gethostbyname(_host);
	if (target_host == NULL) {
		perror("gethostbyname");
		throw ready;
	}
	memcpy(&host, target_host, sizeof(struct hostent));

	port = _port;
*/
	if (sem_init(&queued_fds_count, SOCKET_SEM_PRIVATE, 0) < 0) {
		perror("sem_init");
		throw ready;
	}

	if ((out_fd = connect_to_server()) < 0) {
		throw ready;
	}
	
	ready = true;
}

OutSocketHandler::~OutSocketHandler() {
	if (ready) {
		freeaddrinfo(out_info);
		sem_destroy(&queued_fds_count);
	}
}

int OutSocketHandler::connect_to_server() {
	int fd = socket(out_info->ai_family, out_info->ai_socktype, out_info->ai_protocol);
	if (fd < 0) {
		perror("socket");
		return -1;
	}
	if (connect(fd, out_info->ai_addr, out_info->ai_addrlen) < 0) {
		perror("connect");
		close(fd);
		return -1;
	}
	return fd;
}

void OutSocketHandler::start_loop() {
	int fd, out_fd;
	SockResponse *response;
	unsigned char buf[SOCKET_BUF_SIZE];
	ssize_t bytes_read, bytes_sent;

	while (1) {
		// wait for task
		sem_wait(&queued_fds_count);
		fd = queued_fds.front();
		queued_fds.pop_front();
		response = fd_map[fd];
		dprintf("OutSocketHandler: got fd %d.\n", fd);

		// read message
		Stream *in_stream = new Stream();
		while ((bytes_read = recv(fd, buf, SOCKET_BUF_SIZE, 0)) > 0) {
			in_stream->append(buf, bytes_read);
		}
		#ifdef _DEBUG
		in_stream->dump();
		#endif
		
		Stream *out_response = NULL;
		out_fd = connect_to_server();
		if (out_fd >= 0) {
			bytes_sent = send(out_fd, in_stream->data, in_stream->len, 0);
			dprintf("\033[92mOutSocketHandler: sent %lu bytes.\033[0m\n", bytes_sent);
			
			out_response = new Stream();
			while ((bytes_read = recv(out_fd, buf, SOCKET_BUF_SIZE, 0)) > 0) {
				dprintf("\033[92mOutSocketHandler: got %lu bytes from outflow.\033[0m\n", bytes_read);
				out_response->append(buf, bytes_read);
				// but what if they are equal?
				// the thread will still get blocked.
				if (bytes_read < SOCKET_BUF_SIZE) break;
				else {
					bytes_read = recv(out_fd, buf, SOCKET_BUF_SIZE, MSG_DONTWAIT);
					if (bytes_read == 0) break;
					else out_response->append(buf, bytes_read);
				}
			}
			#ifdef _DEBUG
			dprintf("\033[91mOutSocketHandler: got response:\n");
			out_response->dump();
			dprintf("\033[0m\n");
			#endif

			close(out_fd);
		}

		delete in_stream;

		// work on it
		if (out_response != NULL) {
			response->data = out_response;
		} else {
			// I am a dummy message
			out_response = new Stream;
			out_response->append((void *)"Message received.\n", 19);
			response->data = out_response;
		}

		// mark completed
		sem_post(&response->is_finished);
		done_fds.push_back(fd);
		sem_post(&done_fds_count);
		dprintf("OutSocketHandler: finished work on fd %d.\n", fd);
	}
}
