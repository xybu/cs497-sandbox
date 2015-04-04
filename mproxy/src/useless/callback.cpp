/**
 * callback.cpp
 * libevent callback functions.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <loci/loci.h>
#include <loci/of_message.h>
#include <loci/of_object.h>
#include "global.h"
#include "callback.h"
#include "client_detail.h"
#include "stream.h"

#define OFP_HEADER_BYTES		(8)
#define OFP_HADER_LEN_OFFSET	(2)

void start_event(void *arg) {
	event_base_dispatch(((ClientDetail *)arg)->ev_base);
	dprintf("\33[93mevent loop for fd %d stopped.\033[0m\n", ((ClientDetail *)arg)->fd);
	delete (ClientDetail *)arg;
}

void on_flow_read(struct bufferevent *bev, void *arg) {
	ssize_t bytes_to_read;
	struct evbuffer *input;
	unsigned short len;
	unsigned char *data;
	of_object_t *obj;
	Stream *msg = ((ClientDetail *)arg)->in_buf;
	int fw_fd;
	ClientDetail *fw_cli;
	dprintf("inflow read callback\n");
	input = bufferevent_get_input(bev);
	bytes_to_read = evbuffer_get_length(input);
	// return if the header has not arrived completely
	if (bytes_to_read < OFP_HEADER_BYTES) {
		erprintf("\033[91mon_flow_read: insufficient data to read.\n\033[0m");
		return;
	}
	if (!msg) {
		// allocate memory for a new buffer
		msg = new Stream(bytes_to_read);
		if (!msg) {
			erprintf("\033[91mon_flow_read: cannot malloc.\n\033[0m");
			return;
		}
	} else {
		// expand previous buffer
		if (!msg->expand(bytes_to_read)) {
			erprintf("\033[91mon_flow_read: cannot realloc.\n\033[0m");
			return;
		}
	}
	// read the buffer
	evbuffer_remove(input, msg->data + msg->len, bytes_to_read);
	msg->len += bytes_to_read;
	// get forward client
	fw_fd = ((ClientDetail *)arg)->fw_fd;
	if ((fw_cli = client_map[fw_fd]) == NULL) {
		event_base_loopexit(((ClientDetail *)arg)->ev_base, NULL);
		erprintf("\033[91mon_flow_read: pair has closed. exit client.\n\033[0m");
		return;
	}
	// assume the buffer always starts with an intact header
	data = msg->data;
	while (1) {
		len = *((short *)(data + OFP_HADER_LEN_OFFSET));
		dprintf("msg len = %d\n", len);
		if (len > msg->len) break;
		obj = of_object_new_from_message(((of_message_t)data), len);
		if (!obj) {
			erprintf("\033[91mon_flow_read: NULL of_object_t pointer.\n\033[0m");
			return;
		}
		evbuffer_add(((ClientDetail *)arg)->ev_buf, OF_OBJECT_TO_WBUF(obj), WBUF_CURRENT_BYTES(OF_OBJECT_TO_WBUF(obj)));
		if (bufferevent_write_buffer(((ClientDetail *)arg)->ev_event, ((ClientDetail *)arg)->ev_buf)) {
			erprintf("failed to write to fd %d.\n", ((ClientDetail *)arg)->fd);
		}
		data += len;
		if (data > msg->data + msg->len) break;
	}
	if (data < msg->data + msg->len) {
		memcpy(msg->data, data, msg->data + msg->len - data);
		msg->len = msg->data + msg->len - data;
		((ClientDetail *)arg)->in_buf = msg;
	} else {
		((ClientDetail *)arg)->in_buf = NULL;
		delete msg;
	}
}

void on_flow_error(struct bufferevent *bev, short what, void *arg) {
	dprintf("\033[91mAn error occurred on socket: fd=%d, reason=%d.\033[0m\n", ((ClientDetail *)arg)->fd, what);
}
