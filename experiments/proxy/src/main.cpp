/**
 * main.cpp
 * Main function of the program.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <thread>
#include "global.h"
#include "main.h"
#include "proxy.h"

unsigned int port_number = DEFAULT_LOCAL_PORT;
unsigned int rc_port_number = DEFAULT_RC_PORT;
char rc_host_name[HOST_NAME_MAX + 1] = {0};
char *profile_path = NULL;

void print_usage(const char *prog_name) {
	printf("usage: %s [-p PORT] [-t HOST:PORT] [-f PROFILE] [-h]\n" \
			"\noptional arguments:\n\n" \
			"  -p PORT, --port PORT            \tport number to use.\n" \
			"                                  \tdefault: %d.\n" \
			"  -t HOST:PORT, --target HOST:PORT\taddress to the remote controller.\n" \
			"                                  \tdefault: 127.0.0.1:6634\n" \
			"  -f PROFILE, --profile PROFILE   \tthe protocol profile to load.\n" \
			"                                  \tdefault: omit\n" \
			"  -h, --help                      \tshow this help message and exit.\n", 
			prog_name, DEFAULT_LOCAL_PORT);
}

void parse_port_number(char *arg) {
	port_number = atoi(arg);
	if (port_number > MAX_PORT) {
		die("Invalid port number \"%u\".\n", port_number);
	}
}

void parse_target_addr(char *arg) {
	char *delim = strchr(arg, ':');
	if (delim == NULL) {
		die("Invalid target address \"%s\": port is missing.\n", arg);
	}
	*delim = '\0';
	memcpy(rc_host_name, arg, delim - arg);
	rc_port_number = atoi(delim + 1);
	if (rc_port_number > MAX_PORT) {
		die("Invalid target port number \"%u\".\n", rc_port_number);
	}
}

void parse_profile_path(char *arg) {
	profile_path = arg;
}

int main(int argc, char **argv) {
	int i;
	char **tmp;

	// parse command-line arguments
	for (i = 0, tmp = argv; i < argc; ++i, ++tmp) {
		if (!memcmp(*tmp, "-h", 2)) {
			print_usage(argv[0]);
			return 0;
		} else if (!memcmp(*tmp, "-p", 3) || !memcmp(*tmp, "--port", 7)) {
			++tmp, ++i;
			parse_port_number(*tmp);
		} else if (!memcmp(*tmp, "--port=", 7)) {
			parse_port_number(*tmp + 7);
		} else if (!memcmp(*tmp, "-t", 3) || !memcmp(*tmp, "--target", 9)) {
			++tmp, ++i;
			parse_target_addr(*tmp);
		} else if (!memcmp(*tmp, "--target=", 9)) {
			parse_target_addr(*tmp + 9);
		} else if (!memcmp(*tmp, "-f", 3) || !memcmp(*tmp, "--profile", 10)) {
			++tmp, ++i;
			parse_profile_path(*tmp);
		} else if (!memcmp(*tmp, "--profile=", 10)) {
			parse_profile_path(*tmp + 10);
		}
	}

	// load default host name for remote controller, if missing
	if (rc_host_name[0] == '\0') {
		memcpy(rc_host_name, DEFAULT_RC_HOST, DEFAULT_RC_HOST_LEN);
	}

	dprintf("port number: %u\n", port_number);
	dprintf("target addr: %s:%u\n", rc_host_name, rc_port_number);
	dprintf("load profile: %s\n", profile_path);

	Proxy proxy(port_number);
	proxy.set_server(rc_host_name, rc_port_number);
	proxy.process_requests();

	//std::thread upstream_thread(&UpstreamHandler::process_requests, &upstream_handler);
	//upstream_thread.join();

	return 0;
}
