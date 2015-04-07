#include <loci/loci.h>
#include <loci/of_message.h>
#include <loci/of_object.h>
#include <loci/loci_show.h>
#include <stdarg.h>
#include "global.h"
#include "action.h"

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
	of_object_t *obj = of_object_new_from_message(to_of_message_t(data), len);
	of_wire_buffer_t *wbuf = OF_OBJECT_TO_WBUF(obj);
	unsigned char *buf = WBUF_BUF(wbuf);
	size_t current_bytes = WBUF_CURRENT_BYTES(wbuf);

	#ifdef _DEBUG
	fprintf(stderr, COLOR_GREEN "OpenFlow Message:\n" COLOR_BLACK);
	of_object_show(&of_object_writer, NULL, obj);
	fputc('\n', stderr);
	#endif
	
	stream_t *ret = stream_new(current_bytes);
	stream_append(ret, buf, current_bytes);
	return ret;
}
