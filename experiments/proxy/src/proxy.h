#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class Proxy {
	struct sockaddr_in self_addr;
	int self_fd;
	struct hostent *server;
	struct sockaddr_in server_addr;
public:
	Proxy(int port);
	~Proxy(void);
	void set_server(char *hostname, int port);
	int get_server_socket();
	void process_requests(void);
};
