/**
 * main.cpp
 * The main function.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstring>
#include <climits>
#include <thread>
#include "global.h"
#include "main.h"
#include "proxy.h"

void print_usage(const char *prog_name) {
	printf("usage: %s [-p PORT] [-t HOST:PORT] [-f PROFILE] [-h]\n" \
			"\noptional arguments:\n\n" \
			"  -p PORT, --port PORT            \tport number to use.\n" \
			"                                  \tdefault: %d.\n" \
			"  -t HOST:PORT, --target HOST:PORT\taddress to the remote controller.\n" \
			"                                  \tdefault: %s:%d\n" \
			"  -f PROFILE, --profile PROFILE   \tthe protocol profile to load.\n" \
			"                                  \tdefault: omit\n" \
			"  -h, --help                      \tshow this help message and exit.\n", 
			prog_name, DEFAULT_PORT, DEFAULT_FW_HOST, DEFAULT_FW_PORT);
}

void parse_port_num(char *arg, unsigned int *val) {
	*val = atoi(arg);
	if (!is_valid_port(*val)) {
		die("Invalid port number \"%u\".\n", *val);
	}
	dprintf("Parsed port: %u.\n", *val);
}

void parse_fw_host(char *arg, char *hostname, unsigned int *port) {
	char *delim = strchr(arg, ':');
	if (delim == NULL) {
		die("Invalid forward address \"%s\": port is missing.\n", arg);
	}
	*delim = '\0';
	memcpy(hostname, arg, delim - arg);
	dprintf("Parsed hostname: \"%s\".\n", hostname);
	parse_port_num(delim + 1, port);
}

void parse_profile(char *arg) {

}

int main(int argc, char **argv) {
	int i;
	unsigned int port = DEFAULT_PORT, fw_port = DEFAULT_FW_PORT;
	char fw_host[HOST_NAME_MAX + 1] = DEFAULT_FW_HOST;
	char **tmp, *arg;

	// parse command-line arguments
	for (i = 0, tmp = argv; i < argc; ++i, ++tmp) {
		arg = *tmp;
		if (!memcmp(arg, "-h", 2)) {
			print_usage(argv[0]);
			return 0;
		} else if (!memcmp(arg, "-p", 3) || !memcmp(arg, "--port", 7)) {
			++tmp, ++i;
			arg = *tmp;
			parse_port_num(arg, &port);
		} else if (!memcmp(arg, "--port=", 7)) {
			parse_port_num(arg + 7, &port);
		} else if (!memcmp(arg, "-t", 3) || !memcmp(arg, "--target", 9)) {
			++tmp, ++i;
			arg = *tmp;
			parse_fw_host(arg, fw_host, &fw_port);
		} else if (!memcmp(arg, "--target=", 9)) {
			parse_fw_host(arg + 9, fw_host, &fw_port);
		} else if (!memcmp(arg, "-f", 3) || !memcmp(arg, "--profile", 10)) {
			++tmp, ++i;
			arg = *tmp;
			parse_profile(arg);
		} else if (!memcmp(arg, "--profile=", 10)) {
			parse_profile(arg + 10);
		}
	}

	dprintf("port: %u\n", port);
	dprintf("fw: %s:%u\n", fw_host, fw_port);
	//dprintf("load profile: %s\n", profile_path);

	Proxy proxy(port, fw_host, fw_port);
	std::thread proxy_thread(&Proxy::start, &proxy);

	proxy_thread.join();

	return 0;
}
