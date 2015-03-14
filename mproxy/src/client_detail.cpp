/**
 * client_detail.cpp
 * port-Client map and Client definition.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <unistd.h>
#include <event2/event.h>
#include <event2/util.h>
#include "global.h"
#include "client_detail.h"

// global fd-client map
std::unordered_map<int, ClientDetail *> client_map;

ClientDetail::ClientDetail(int cli_fd, int forward_fd) {
	dprintf("ClientDetail: creating pair (%d, %d).\n", cli_fd, forward_fd);
	fd = cli_fd;
	fw_fd = forward_fd;
	ev_base = NULL;
	ev_buf = NULL;
	ev_buf = NULL;
}

ClientDetail::~ClientDetail() {
	ClientDetail *fw_cli = client_map[fw_fd];
	if (ev_event) {
		bufferevent_free(ev_event);
		ev_event = NULL;
	}
	if (ev_base){
		event_base_free(ev_base);
		ev_base = NULL;
	}
	if (ev_buf) {
		evbuffer_free(ev_buf);
		ev_buf = NULL;
	}
	if (fd != -1) close(fd);
	if (client_map.find(fd) != client_map.end()) {
		client_map.erase(fd);
	}
	if (fw_cli != NULL) {
		// mark pair's reference to this NULL
		fw_cli->fw_fd = -1;
		// try to stop forward port as well
		event_base_loopbreak(fw_cli->ev_base);
	}
	dprintf("A ClientDetail object was freed.\n");
}

int ClientDetail::init(bufferevent_data_cb on_read, bufferevent_data_cb on_write, bufferevent_event_cb on_error) {

	if (!(ev_buf = evbuffer_new())) {
		erprintf("failed to create event buffer.\n");
		return STATUS_ERR;
	}

	if (!(ev_base = event_base_new())) {
		erprintf("failed to create event base.\n");
		return STATUS_ERR;
	}

	if (!(ev_event = bufferevent_socket_new(ev_base, fd, 0))) {
		erprintf("failed to create buffered event.\n");
		return STATUS_ERR;
	}

	bufferevent_setcb(ev_event, on_read, on_write, on_error, this);
	bufferevent_base_set(ev_base, ev_event);
	//bufferevent_set_timeouts(ev_event, SOCK_READ_TIMEOUT, SOCK_WRITE_TIMEOUT);
	bufferevent_enable(ev_event, EV_READ);

	client_map[fd] = this;

	return STATUS_OK;
}
