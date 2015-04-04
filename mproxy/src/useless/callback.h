/**
 * callback.h
 * libevent callback function declarations.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _CALLBACK_H

#define _CALLBACK_H

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

void start_event(void *);
void on_flow_read(struct bufferevent *bev, void *arg);
void on_flow_error(struct bufferevent *bev, short what, void *arg);

#endif
