#ifndef _PROXY_H

#define _PROXY_H

#include <atomic>
#include <deque>
#include <thread>
#include <unordered_map>
#include <semaphore.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "stream.h"

#define SEM_PRIVATE				0
#define SEM_SHARED				1
#define DEF_SOCKET_BUF_SIZE		384
#define DEF_INSOCK_QUEUE_LEN	15

// dummy typedef
#define SockAddr6				struct sockaddr_in6
#define ClientId				unsigned int
#define ProxyStatus				int

typedef struct client_detail {
	ClientId id;
	int in_fd;
	int out_fd;
	sem_t in_sem;
	sem_t out_sem;
	SockAddr6 addr;
} ClientDetail;

#define PROXY_STATUS_ERR		-1
#define PROXY_STATUS_OK			0

/**
 * A multi-threaded proxy class
 * 
 * For every connection, a thread will be created to handle the communication
 * There will also be a scheduler thread and a listener thread.
 */
class Proxy {
private:
	// count the total number of clients
	std::atomic_uint client_count;

	// pool of clients. Key is assigned id
	std::unordered_map<ClientId, ClientDetail *> client_pool;

	// the packet order should be preserved
	// so when a thread is ready to go, put it to ready queue
	// so the scheduler can activate it
	// std::deque<ThreadId> thread_ready_queue;
	std::deque<std::thread *> thread_pool;

	// when a thread puts itself in ready queue, it should wake up
	// the scheduler thread by posting this semaphore
	sem_t scheduler_sem;
	std::thread *scheduler;

	// listener thread
	std:: thread *listener;

	// status variable
	ProxyStatus status;

	// in socket
	int in_sock_fd;
	SockAddr6 in_sock;

	// out socket
	struct addrinfo *out_host;

public:
	Proxy(int port, char *fw_host, int fw_port);
	ProxyStatus init_in_socket(int port, int queue_len = DEF_INSOCK_QUEUE_LEN);
	ProxyStatus init_out_socket(char *fw_host, int fw_port);
	void init_scheduler();
	void init_listener();
	int get_outflow_fd();
	void start();
	void run_listener();
	void run_scheduler();
	void run_inflow_handler(ClientDetail *detail);
	void run_outflow_handler(ClientDetail *detail);
	Stream *read_message(int fd);
	Stream *prepare_in_request(Stream *msg);
	Stream *prepare_in_response(Stream *msg);
	Stream *prepare_out_request(Stream *msg);
	Stream *prepare_out_response(Stream *msg);
	~Proxy();
};

#endif
