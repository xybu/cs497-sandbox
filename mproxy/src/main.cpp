/**
 * main.cpp
 * The main function.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <climits>
#include <csignal>
#include <thread>
#include <event2/event.h>
#include <event2/thread.h>
#include "global.h"
#include "main.h"
#include "proxy.h"
#include "task.h"
#include "worker.h"
#include "client.h"

#ifdef _WIN32
 	#define LIBEVENT_THREAD_INIT	evthread_use_windows_threads
#else
 	#define LIBEVENT_THREAD_INIT	evthread_use_pthreads
#endif

Proxy *proxy = NULL;

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
		die(1, "Invalid port number \"%u\".\n", *val);
	}
	dbg("Parsed port: %u.\n", *val);
}

void parse_fw_host(char *arg, char *hostname, unsigned int *port) {
	char *delim = strchr(arg, ':');
	if (delim == NULL) {
		die(1, "Invalid forward address \"%s\": port is missing.\n", arg);
	}
	*delim = '\0';
	memcpy(hostname, arg, delim - arg);
	dbg("Parsed hostname: \"%s\".\n", hostname);
	parse_port_num(delim + 1, port);
}

void parse_profile(char *arg) {

}

void terminate_main(int sig) {
	dbg(COLOR_CYAN "Stopping program...\n" COLOR_BLACK);
	if (proxy) {
		proxy->stop();
	}
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

	if (LIBEVENT_THREAD_INIT()) {
		die(1, "main: failed to initialize libevent.\n");
	}

	if (signal(SIGTERM, terminate_main) == SIG_ERR ||
		signal(SIGINT, terminate_main) == SIG_ERR) {
		die(1, "main: failed to register signal handler.\n");
	}

	dbg("port: %u\n", port);
	dbg("fw: %s:%u\n", fw_host, fw_port);
	//dbg("load profile: %s\n", profile_path);

	// instantiate task queue
	task_queue = new TaskQueue();

	// start workers
	Worker *workers[NUM_OF_WORKERS];
	for (i = 0; i < NUM_OF_WORKERS; ++i) {
		Worker *w = new Worker(i);
		w->start();
		workers[i] = w;
	}
	log(COLOR_CYAN "main: all workers started.\n" COLOR_BLACK);

	// instantiate proxy
	try {
		proxy = new Proxy(port, fw_host, fw_port);
	} catch (int err) {
		log(COLOR_RED "main: failed to start proxy.\n" COLOR_BLACK);
		exit(err);
	}
	log(COLOR_CYAN "main: starting proxy...\n" COLOR_BLACK);
	proxy->start();

	// cleanup client detail objects
	log(COLOR_CYAN "main: housekeeping...\n" COLOR_BLACK);
	dbg("main: sweeping client detail objects (%lu total)...\n", client_map.size());
	while (client_map.size() > 0) {
		client_map.erase(-1);
		auto it = client_map.begin();
		if (it == client_map.end()) break;
		Client *d = it->second;
		if (d && d->ev_base) {
			dbg("main: I try to break loop for fd %d.\n", d->fd);
			event_base_loopbreak(d->ev_base);
		} else if (d) {
			dbg("main: I got fd %d.\n", d->fd);
		} else {
			dbg("main: I got a NULL whose index is %d.\n", it->first);
		}
		dbg("main: there are %lu objects in map.\n", client_map.size());
		sleep(1);
	}
	
	// stop workers
	dbg("main: removing workers...\n");
	for (i = 0; i < NUM_OF_WORKERS; ++i) {
		workers[i]->can_run = false;
	}
	task_queue->increment_count(NUM_OF_WORKERS);
	for (i = 0; i < NUM_OF_WORKERS; ++i) {
		dbg("main: waiting for worker%d.\n", i);
		workers[i]->th->join();
		delete workers[i];
	}
	delete task_queue;

	// free proxy
	delete proxy;

	// It is known that libevent2 has memory leak when 
	// evthread_use_pthread() is called.
	// libevent_global_shutdown();
	
	log(COLOR_CYAN "Program exit.\n" COLOR_BLACK);

	return 0;
}
