#ifndef _OUT_SOCKET_H

#define _OUT_SOCKET_H

#include <netdb.h>
#include <sys/socket.h>

class OutSocketHandler {
	bool ready;
	struct addrinfo *out_info;
	int out_fd;
public:
	OutSocketHandler(char *_host, int _port);
	~OutSocketHandler();
	int connect_to_server();
	void start_loop();
};

#endif
