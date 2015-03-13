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
	
//#define _DEBUG
//#define _COLORFUL

#define	die(format, args...);	fprintf(stderr, format, ##args);exit(1);

#define erprintf(format, args...);	fprintf(stderr, format , ##args);

#ifdef _DEBUG
	#define	dprintf(format, args...);	fprintf(stderr, format , ##args);
#else
	#define	dprintf(format, args...);
#endif

#ifdef _COLORFUL
	#define pperror(x);	{fprintf(stderr, "\033[91m");perror(x);fprintf(stderr, "\033[0m");}
#else
	#define pperror(x);	perror(x);
#endif

#endif
