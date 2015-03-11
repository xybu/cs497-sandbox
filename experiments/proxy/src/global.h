/**
 * global.h
 * Globally used constants and declarations.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#define _DEBUG

#define	die(format, args...);	fprintf(stderr, format, ##args);exit(1);

#ifdef _DEBUG
	#define	dprintf(format, args...);	fprintf(stderr, format , ##args);
#else
	#define	dprintf(format, args...);
#endif

#define erprintf(format, args...);	fprintf(stderr, format , ##args);

