/**
 * global.h
 * Globally used constants and declarations.
 * The header is included in both C++ and C source files.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _MPROXY_GLOBAL_H

#define _MPROXY_GLOBAL_H

//#define _DEBUG
//#define _COLORFUL

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

#define	die(x, ...)			{fprintf(stderr, __VA_ARGS__);exit(x);}
#define err(...)			fprintf(stderr, __VA_ARGS__)
#define log(...)			fprintf(stderr, __VA_ARGS__)
#define is_valid_fd(x)		(fcntl(x,  F_GETFL) != -1)

#ifdef _COLORFUL
 	#define COLOR_RED		"\033[91m"
	#define COLOR_GREEN		"\033[92m"
	#define COLOR_YELLOW	"\033[93m"
	#define COLOR_CYAN		"\033[96m"
	#define COLOR_BLACK		"\033[0m"
	#define pperror(x);	{fprintf(stderr, COLOR_RED);perror(x);fprintf(stderr, COLOR_BLACK);}
#else
 	#define COLOR_RED	
	#define COLOR_GREEN	
	#define COLOR_YELLOW
	#define COLOR_CYAN	
	#define COLOR_BLACK
	#define pperror(x)	perror(x)
#endif

#ifdef _DEBUG
	#define	dbg(...)	fprintf(stderr, __VA_ARGS__)
#else
	#define	dbg(...);
#endif

#endif
