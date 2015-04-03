/**
 * client_detail.h
 * port-Client map and Client definition.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _CLIENT_DETAIL_H

#define _CLIENT_DETAIL_H

#include <unordered_map>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "stream.h"

class ClientDetail {
public:
	int fd;
	int fw_fd;
	struct event_base *ev_base;
	struct evbuffer	*ev_buf;
	struct bufferevent *ev_event;
	Stream *in_buf;
	
	ClientDetail(int, int);
	~ClientDetail();
	int init(bufferevent_data_cb on_read, bufferevent_data_cb on_write, bufferevent_event_cb on_error);
};

extern std::unordered_map<int, ClientDetail *> client_map;

#endif
