#include <stdarg.h>
#include <stdlib.h>
#include <loci/loci.h>
#include <loci/of_message.h>
#include <loci/of_object.h>
#include <loci/loci_show.h>
#include "global.h"
#include "action.h"
#include "attack.h"

#define to_of_message_t(x)	((of_message_t)x)

int of_object_writer(void *cookie, const char *fmt, ...) {
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = vfprintf(stderr, fmt, args);
	va_end(args);
	return ret;
}

stream_t *action_inject(unsigned char *data, uint16_t len) {
	of_object_t *obj;
	of_wire_buffer_t *wbuf;
	size_t current_bytes;
	unsigned char *buf;
	stream_t *ret;
	unsigned int ofp_ver;
	unsigned int msg_type;
	unsigned int rnd, ptmp;

	ofp_ver = data[0];
	msg_type = data[1];
	
	if (!is_valid_ofp_header(ofp_ver, msg_type)) {
		err(COLOR_RED "Got a msg that has invalid version-type pair: (%u, %u).\n" COLOR_BLACK, ofp_ver, msg_type);
		return NULL;
	}

	if (should_drop_msg(ptmp, ofp_ver, msg_type)) {
		rnd = rand() % 100;
		if (rnd < ptmp) {
			log(COLOR_CYAN "Message of OFP %d, type %d has been dropped. Rand=%d.\n" COLOR_BLACK, ofp_ver, msg_type, rnd);
			return NULL;
		}
	}

	obj = of_object_new_from_message(to_of_message_t(data), len);
	wbuf = OF_OBJECT_TO_WBUF(obj);
	buf = WBUF_BUF(wbuf);
	current_bytes = WBUF_CURRENT_BYTES(wbuf);

	#ifdef _DEBUG
	fprintf(stderr, COLOR_GREEN "OpenFlow Message:\n" COLOR_BLACK);
	of_object_show(&of_object_writer, NULL, obj);
	fputc('\n', stderr);
	#endif
	
	ret = stream_new(current_bytes);
	stream_append(ret, buf, current_bytes);
	return ret;
}
