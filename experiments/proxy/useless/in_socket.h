#ifndef _IN_SOCKET_H

#define _IN_SOCKET_H

#include <thread>
#include <netinet/in.h>

#define INSOCKET_QUEUE_LEN		15
#define INSOCKET_NUM_WORKERS	2

class InSocketHandler {
	bool ready;
	int in_sock_fd;
	struct sockaddr_in6 in_sock;
	sem_t count;
	std::thread workers[INSOCKET_NUM_WORKERS];
public:
	InSocketHandler(int port, int queue_len=INSOCKET_QUEUE_LEN);
	~InSocketHandler();
	void respond(int id);
	void start_loop();
};

#endif