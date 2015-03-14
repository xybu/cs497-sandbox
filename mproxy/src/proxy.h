/**
 * proxy.h
 * Declared proxy datatypes.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _PROXY_H

#define _PROXY_H

#include <deque>
#include <thread>
#include <errno.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define DEF_INSOCK_QUEUE_LEN	15

// dummy typedef
#define SockAddr6				struct sockaddr_in6
#define ProxyStatus				int

// inline functions
#define is_valid_fd(x)		(fcntl(x,  F_GETFL) != -1)

/**
 * A multi-threaded proxy class
 * 
 * For every connection, a thread will be created to handle the communication.
 */
class Proxy {
private:
	ProxyStatus status;			// indicate object status
	int in_sock_fd;				// incoming socket
	SockAddr6 in_sock;			// local socket address info
	struct addrinfo *out_host;	// forward host info
public:
	std::deque<std::thread *> handler_pool;

	Proxy(int port, char *fw_host, int fw_port);
	ProxyStatus init_in_socket(int port, int queue_len = DEF_INSOCK_QUEUE_LEN);
	ProxyStatus init_out_socket(char *fw_host, int fw_port);
	void init_scheduler();
	void init_listener();
	int get_outflow_fd();
	void start();
	void start_listen();
	void stop();
	~Proxy();
};

#endif
