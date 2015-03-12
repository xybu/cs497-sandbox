#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include "global.h"
#include "stream.h"
#include "proxy.h"

#define SOCKET_QUEUE_LEN	10
#define SOCKET_BUF_SIZE		256

Proxy::Proxy(int port) {
	self_fd = -1;
	memset(&self_addr, 0, sizeof(struct sockaddr_in));
	int optval = 1;
	self_addr.sin_family = AF_INET;
	self_addr.sin_addr.s_addr = INADDR_ANY;
	self_addr.sin_port = htons((u_short)port);
	self_fd = socket(PF_INET, SOCK_STREAM, 0);

	if (self_fd < 0) {
		perror("socket");
		die("Proxy: init failed.\n");
	}

	if (setsockopt(self_fd, SOL_SOCKET, 
		SO_REUSEADDR, &optval, sizeof(int))) {
		perror("setsockopt");
		die("Proxy: init failed.\n");
	}

	if (bind(self_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr_in))) {
		perror("bind");
		die("Proxy: init failed.\n");
	}

	if (listen(self_fd, SOCKET_QUEUE_LEN)) {
		perror("listen");
		die("Proxy: init failed.\n");
	}
	
	dprintf("Proxy: object initialized.\n");
}

Proxy::~Proxy() {
	// close is declared in <unistd.h>
	if (self_fd >= 0) close(self_fd);
	
	dprintf("Proxy: object freed.\n");
}

void Proxy::set_server(char *hostname, int port) {
	server = gethostbyname(hostname);
	if (!server) {
		perror("gethostbyname");
		die("Proxy: controller host not found.");
	}
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	server_addr.sin_port = htons((u_short)port);
	if (inet_pton(AF_INET, , &serv_addr.sin_addr) <= 0) {
		
	}
}

int Proxy::get_server_socket() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}
	if (connect(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		close(fd);
		return -1;
	}
	dprintf("get_server_socket: server socket %d.\n", fd);
	return fd;
}

void Proxy::process_requests() {
	socklen_t socket_len = sizeof(struct sockaddr_in);
	
	while (1) {
		struct sockaddr_in cli_addr;
		int cli_socket;

		dprintf("Proxy: waiting for requests.\n");
		cli_socket = accept(self_fd, (struct sockaddr *)&cli_addr, &socket_len);
		if (cli_socket < 0){
			dprintf("Proxy: negative client socket.\n");
			continue;
		}
		dprintf("Proxy: received one request.\n");
		
		// assume single threading for now
		unsigned char buf[SOCKET_BUF_SIZE];
		size_t bytes_read = 0;
		Stream *cli_stream = new Stream();
		while ((bytes_read = recv(cli_socket, buf, SOCKET_BUF_SIZE, 0)) > 0) {
			cli_stream->append(buf, bytes_read);
		}
		cli_stream->dump();

		int server_socket = get_server_socket();
		if (server_socket >= 0) {
			send(server_socket, cli_stream->data, cli_stream->len, 0);
			Stream *server_response = new Stream();
			while ((bytes_read = recv(server_socket, buf, SOCKET_BUF_SIZE, 0)) > 0) {
				server_response->append(buf, bytes_read);
			}
			server_response->dump();
			send(cli_socket, server_response->data, server_response->len, 0);
			delete server_response;
			close(server_socket);
		}

		// send(cli_socket, stream->data, stream->len, 0);

		delete cli_stream;
		close(cli_socket);
	}
}
