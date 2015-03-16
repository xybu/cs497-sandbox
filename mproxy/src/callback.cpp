/**
 * callback.cpp
 * libevent callback functions.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include "global.h"
#include "callback.h"
#include "client_detail.h"
#include "ofp.h"
#include "stream.h"

namespace callback {

	static Stream *read_message(struct bufferevent *bev) {
		ssize_t bytes_to_read;
		Stream *msg = NULL;
		struct evbuffer *input = bufferevent_get_input(bev);
		bytes_to_read = evbuffer_get_length(input);

		if (bytes_to_read > 0) {
			msg = new Stream(bytes_to_read);
			evbuffer_remove(input, msg->data, bytes_to_read);
			msg->len = bytes_to_read; 
		}

		#ifdef _DEBUG
		if (msg) {
			//dprintf("read_message:\n");
			//msg->dump();
		} else {
			dprintf("message is NULL.\n");
		}
		#endif

		return msg;
	}

	static void write_message(Stream *msg, ClientDetail *client) {
		dprintf("write_message: msg = %p, fd = %d.\n", msg, client->fd);
		evbuffer_add(client->ev_buf, msg->data, msg->len);
		if (bufferevent_write_buffer(client->ev_event, client->ev_buf)) {
			erprintf("failed to write to fd %d.\n", client->fd);
		}
	}

	void start_event(void *arg) {
		event_base_dispatch(((ClientDetail *)arg)->ev_base);
		dprintf("\33[93mevent loop for fd %d stopped.\033[0m\n", ((ClientDetail *)arg)->fd);
		delete (ClientDetail *)arg;
	}

	void on_inflow_read(struct bufferevent *bev, void *arg) {
		dprintf("inflow read callback\n");
		Stream *msg = read_message(bev);
		int fw_fd = ((ClientDetail *)arg)->fw_fd;
		//dprintf("on_inflow_read = %d.\n", fw_fd);
		ClientDetail *fw_cli = client_map[fw_fd];
		if (fw_cli == NULL) {
			event_base_loopexit(((ClientDetail *)arg)->ev_base, NULL);
		} else {
			write_message(msg, fw_cli);
		}
		delete msg;
	}

	void on_outflow_read(struct bufferevent *bev, void *arg){
		dprintf("outflow read callback\n");
		Stream *msg = read_message(bev);
		int fw_fd = ((ClientDetail *)arg)->fw_fd;
		//dprintf("on_outflow_read = %d.\n", fw_fd);
		ClientDetail *fw_cli = client_map[fw_fd];
		if (fw_cli == NULL) {
			event_base_loopexit(((ClientDetail *)arg)->ev_base, NULL);
		} else {
			write_message(msg, fw_cli);
		}
		delete msg;
	}

	void on_inflow_write(struct bufferevent *bev, void *arg) {
		dprintf("inflow write cb\n");
	}

	void on_flow_error(struct bufferevent *bev, short what, void *arg) {
		dprintf("\033[91mAn error occurred on socket: fd=%d, reason=%d.\033[0m\n", ((ClientDetail *)arg)->fd, what);
	}

	void on_outflow_write(struct bufferevent *bev, void *arg){
		dprintf("outflow write cb\n");
	}
}
