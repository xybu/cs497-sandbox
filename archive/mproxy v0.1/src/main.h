/**
 * main.h
 * Defines constants for the main function.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

// default forward address
#define DEFAULT_PORT		6633
#define DEFAULT_FW_DEV		"lo"
#define DEFAULT_FW_HOST		"127.0.0.1"
#define DEFAULT_FW_PORT		6635

// macro functions
#define is_valid_port(x)	((x >= 0) && ( x <= 65535))
