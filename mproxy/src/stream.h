/**
 * stream.h
 * Class declaration for a simple byte stream wrapper.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _STREAM_H

#define _STREAM_H

#define DEFAULT_STREAM_LEN	512

#ifdef __cplusplus
 	#define EXTERNC extern "C"
#else
	#define EXTERNC
#endif

typedef struct stream_s {
	unsigned char *data;
	size_t cap;
	size_t len;
} stream_t;

#define stream_append2(x, y)	stream_append(x, y->data, y->len)

EXTERNC void memory_dump(unsigned char *p, int len);
EXTERNC stream_t *stream_new(size_t size);
EXTERNC void stream_free(stream_t *s);
EXTERNC int stream_append(stream_t *s, unsigned char *data, size_t len);
EXTERNC int stream_expand(stream_t *s, size_t size);
EXTERNC void stream_left_shift(stream_t *s, size_t size);

#ifdef _DEBUG
EXTERNC void stream_dump(stream_t *s);
#endif

#endif
