/**
 * global.h
 * Globally used constants and declarations.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _MPROXY_GLOBAL_H
	#define _MPROXY_GLOBAL_H
	#define _DEBUG
	#define	die(format, args...);	fprintf(stderr, format, ##args);exit(1);
	#define erprintf(format, args...);	fprintf(stderr, format , ##args);
	
	#ifdef _DEBUG
		#define	dprintf(format, args...);	fprintf(stderr, format , ##args);
	#else
		#define	dprintf(format, args...);
	#endif
#endif
