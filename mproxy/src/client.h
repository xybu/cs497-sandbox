/**
 * client_detail.h
 * Port-Client mapping and Client definition.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _CLIENT_H

#define _CLIENT_H

#include <unordered_map>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <pthread.h>
#include "stream.h"

struct Client {
public:
	int fd;
	int fw_fd;
	pthread_mutex_t write_mutex;
	struct event_base *ev_base;
	struct evbuffer	*ev_buf;
	struct bufferevent *ev_event;
	stream_t *buf;
	
	Client(int, int);
	~Client();
	int init(bufferevent_data_cb on_read, bufferevent_data_cb on_write, bufferevent_event_cb on_error);
};

extern std::unordered_map<int, Client *> client_map;

#endif
