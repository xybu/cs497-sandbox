#include <loci/loci.h>
#include <loci/of_message.h>
#include <loci/of_object.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "stream.h"
#include "action.h"

void act_start_loop(struct event_base *ev_base) {
	event_base_dispatch(ev_base);
}

int act_on_flow_read(struct event_base *ev_base, 
	struct bufferevent *ev_event, 
	struct evbuffer	*ev_buf,
	int fd, int fw_fd, stream_t **bufp) {
	
	register stream_t *buf = *bufp;
	
}
