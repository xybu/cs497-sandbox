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
#include <unistd.h>

#ifndef NUM_OF_WORKERS
 #define NUM_OF_WORKERS 2
#endif

#define STATUS_ERR				-1
#define STATUS_OK				0
#define STATUS_STOP				1

#define SOCK_READ_TIMEOUT		5
#define SOCK_WRITE_TIMEOUT		5

#define SEM_PRIVATE				0
#define SEM_SHARED				1

//#define _DEBUG
//#define _COLORFUL

#define	die(format, args...);	fprintf(stderr, format, ##args);exit(1);

#define erprintf(format, args...)	fprintf(stderr, format , ##args)

#ifdef _DEBUG
	#define	dprintf(format, args...)	fprintf(stderr, format , ##args)
#else
	#define	dprintf(format, args...)
#endif

#ifdef _COLORFUL
	#define pperror(x);	{fprintf(stderr, "\033[91m");perror(x);fprintf(stderr, "\033[0m");}
#else
	#define pperror(x)	perror(x)
#endif

#endif
