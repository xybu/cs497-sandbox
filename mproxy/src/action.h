#ifndef _ACTION_H

#define _ACTION_H

#ifdef __cplusplus
 	#define EXTERNC extern "C"
#else
	#define EXTERNC
#endif

#include "stream.h"

EXTERNC stream_t *action_inject(unsigned char *data, uint16_t len);

#endif
