#include <cstring>
#include "global.h"
#include "proxy.h"

// How do you preserve the order?
// How do you free all resources at exit?

Proxy::Proxy(int port, char *fw_host, int fw_port) {
	status = PROXY_STATUS_ERR;
	client_count = 0;
	in_sock_fd = -1;
	out_host = NULL;
	if (init_in_socket(port) || init_out_socket(fw_host, fw_port)) {
		throw status;
	}
	if (sem_init(&scheduler_sem, SEM_PRIVATE, 0) < 0) {
		pperror("sem_init");
		throw status;
	}
	status = PROXY_STATUS_OK;
}

Proxy::~Proxy() {
	//for (std::deque<std::thread *>::iterator it = thread_pool.begin(); 
	//		it != thread_pool.end(); ++it) {
	//	*it.terminate();
	//}
	//scheduler.terminate();

	if (in_sock_fd >= 0) close(in_sock_fd);
	if (out_host != NULL) freeaddrinfo(out_host);
	if (status == PROXY_STATUS_OK) sem_destroy(&scheduler_sem);
}

ProxyStatus Proxy::init_in_socket(int port, int queue_len) {
	int tmp = 0;
	memset(&in_sock, 0, sizeof(SockAddr6));
	if ((in_sock_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		pperror("socket");
		return PROXY_STATUS_ERR;
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
		return PROXY_STATUS_ERR;
	}
	if (bind(in_sock_fd, (struct sockaddr*)&in_sock, sizeof(struct sockaddr_in6)) < 0) {
		pperror("bind");
		close(in_sock_fd);
		in_sock_fd = -1;
		return PROXY_STATUS_ERR;
	}
	if (listen(in_sock_fd, queue_len) < 0) {
		pperror("listen");
		close(in_sock_fd);
		in_sock_fd = -1;
		return PROXY_STATUS_ERR;
	}
	return PROXY_STATUS_OK;
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
		return PROXY_STATUS_ERR;
	}
	return PROXY_STATUS_OK;
}

void Proxy::init_scheduler() {
	scheduler = new std::thread(&Proxy::run_scheduler, this);
}

void Proxy::init_listener() {
	listener = new std::thread(&Proxy::run_listener, this);
}

int Proxy::get_outflow_fd() {
	int fd = socket(out_host->ai_family, out_host->ai_socktype, out_host->ai_protocol);
	if (fd < 0) {
		pperror("socket");
		return PROXY_STATUS_ERR;
	}
	if (connect(fd, out_host->ai_addr, out_host->ai_addrlen) < 0) {
		pperror("connect");
		close(fd);
		return PROXY_STATUS_ERR;
	}
	return fd;
}

void Proxy::start() {
	init_scheduler();
	//init_listener();
	run_listener();
}

void Proxy::run_listener() {
	register socklen_t cli_sock_len = sizeof(SockAddr6);

	dprintf("listener: started.\n");
	while (1) {
		dprintf("listener: waiting for requests.\n");

		ClientDetail *td = new ClientDetail;
		if (td == NULL) {
			// what if error repeatedly occurs?
			pperror("malloc");
			continue;
		}

		td->in_fd = accept(in_sock_fd, (struct sockaddr *)&td->addr, &cli_sock_len);
		if (td->in_fd < 0){
			dprintf("listener: negative client socket.\n");
			delete td;
			continue;
		}

		dprintf("listener: got request on port %u.\n", td->addr.sin6_port);

		// create a new connection to fw host
		td->out_fd = get_outflow_fd();
		if (td->out_fd < 0) {
			close(td->in_fd);
			delete td;
			continue;
		}

		// prepare worker semaphore
		if (sem_init(&td->in_sem, SEM_PRIVATE, 1) < 0) {
			pperror("sem_init");
			close(td->in_fd);
			close(td->out_fd);
			delete td;
			continue;
		}
		if (sem_init(&td->out_sem, SEM_PRIVATE, 0) < 0) {
			pperror("sem_init");
			close(td->in_fd);
			close(td->out_fd);
			sem_destroy(&td->in_sem);
			delete td;
			continue;
		}

		// assign client id
		td->id = client_count++;
		client_pool[td->id] = td;
		thread_pool.push_back(new std::thread(&Proxy::run_outflow_handler, this, td));
		thread_pool.push_back(new std::thread(&Proxy::run_inflow_handler, this, td));

		dprintf("listener: handled one request.\n");
	}
}

void Proxy::run_scheduler() {
	//ThreadId tid;

	dprintf("scheduler: started.\n");
	while (1) {
		sem_wait(&scheduler_sem);

		// release the thread at the top of ready queue
		// tid = thread_ready_queue.front();
		// thread_ready_queue.pop_front();
		// td = thread_pool[tid];
		// if (td != NULL) sem_post(&td->sem);
	}
}

void Proxy::run_inflow_handler(ClientDetail *detail) {
	register size_t bytes_sent;
	register Stream *msg;
	register int out_fd = detail->out_fd;
	register int in_fd = detail->in_fd;
	register sem_t *in_sem = &detail->in_sem;
	register sem_t *out_sem = &detail->out_sem;
	dprintf("in-handler %u: started.\n", detail->id);

	// new socket connection from a client
	// process hello message
	while (1) {
		sem_wait(in_sem);
		msg = read_message(in_fd);
		msg = prepare_out_request(msg);
		bytes_sent = send(out_fd, msg->data, msg->len, 0);
		if (bytes_sent != msg->len) {
			dprintf("in-handler %u: msg was not sent completely.\n", detail->id);
		}
		delete msg;
		sem_post(out_sem);
	}
}

void Proxy::run_outflow_handler(ClientDetail *detail) {
	register size_t bytes_sent;
	register Stream *msg;
	register int out_fd = detail->out_fd;
	register int in_fd = detail->in_fd;
	register sem_t *in_sem = &detail->in_sem;
	register sem_t *out_sem = &detail->out_sem;
	dprintf("out-handler %u: started.\n", detail->id);

	while (1) {
		sem_wait(out_sem);
		msg = read_message(out_fd);
		msg = prepare_in_response(msg);
		bytes_sent = send(in_fd, msg->data, msg->len, 0);
		if (bytes_sent != msg->len) {
			dprintf("in-handler %u: msg was not sent completely.\n", detail->id);
		}
		delete msg;
		sem_post(in_sem);
	}
}

Stream *Proxy::read_message(int fd) {
	ssize_t bytes_read;
	unsigned char buf[DEF_SOCKET_BUF_SIZE];

	Stream *msg = new Stream();
	while ((bytes_read = recv(fd, buf, DEF_SOCKET_BUF_SIZE, 0)) > 0) {
		msg->append(buf, bytes_read);
		if (bytes_read < DEF_SOCKET_BUF_SIZE) break;

		bytes_read = recv(fd, buf, DEF_SOCKET_BUF_SIZE, MSG_DONTWAIT);
		if (bytes_read <= 0) break;
		msg->append(buf, bytes_read);
	}

	#ifdef _DEBUG
	dprintf("read_message:\n");
	msg->dump();
	#endif

	return msg;
}

Stream *Proxy::prepare_out_request(Stream *msg) {
	// do not modify for now
	return msg;
}

Stream *Proxy::prepare_out_response(Stream *msg) {
	// do not modify for now
	return msg;
}

Stream *Proxy::prepare_in_request(Stream *msg) {
	// do not modify for now
	return msg;
}

Stream *Proxy::prepare_in_response(Stream *msg) {
	// do not modify for now
	return msg;
}
