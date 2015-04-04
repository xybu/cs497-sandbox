#ifndef _ACTION_H

#define _ACTION_H

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#ifdef __cplusplus
 	#define EXTERNC extern "C"
#else
	#define EXTERNC
#endif

#define ACT_SUCCESS			0
#define ACT_ERR_FWCLOSED	1

EXTERNC void act_start_loop(struct event_base *ev_base);
EXTERNC int act_on_flow_read(struct event_base *ev_base, 
	struct bufferevent *ev_event, 
	struct evbuffer	*ev_buf,
	int fd, int fw_fd, stream_t **bufp);

#endif
