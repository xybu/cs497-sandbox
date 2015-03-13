/**
 * global.h
 * Globally used constants and declarations.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _MPROXY_GLOBAL_H

#define _MPROXY_GLOBAL_H

#include <cstdio>
#include <cstdlib>
#include <semaphore.h>
#include <unordered_map>
#include <deque>
#include "stream.h"
	
#define _DEBUG

#define	die(format, args...);	fprintf(stderr, format, ##args);exit(1);

#define erprintf(format, args...);	fprintf(stderr, format , ##args);

#ifdef _DEBUG
	#define	dprintf(format, args...);	fprintf(stderr, format , ##args);
#else
	#define	dprintf(format, args...);
#endif

typedef struct sock_comm {
	sem_t request_ready;
	sem_t response_ready;
	Stream *data;
} SocketComm;

extern std::unordered_map<int, SocketComm *> fd_map;

// managed by in_socket component
extern std::deque<int> done_fds;
extern sem_t done_fds_count;

// managed by out_socket component
extern std::deque<int> queued_fds;
extern sem_t queued_fds_count;

#define SOCKET_SEM_PRIVATE	0
#define SOCKET_SEM_SHARED	1
#define SOCKET_BUF_SIZE		512

#endif
