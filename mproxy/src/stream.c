/**
 * stream.cpp
 * A simple byte stream wrapper.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "stream.h"

#ifdef _DEBUG

#define MAX_DUMP_BYTES 16
#define SMALLEST_VASCII	32
#define LARGEST_VASCII	126

void memory_dump(unsigned char *p, int len) {
	int i, j;
	char buf[MAX_DUMP_BYTES + 1]; // buffer for string representation
	for (i = 0; i < len; ++i){
		if (i % MAX_DUMP_BYTES == 0){ //initialization work
			for (j = 0; j < MAX_DUMP_BYTES; ++j) buf[j] = '.';
			buf[MAX_DUMP_BYTES] = '\0';
			fprintf(stderr, "%p: ", p);
		}
		fprintf(stderr, "%02x ", *p);
		if (*p >= SMALLEST_VASCII && *p <= LARGEST_VASCII) buf[i % MAX_DUMP_BYTES] = *p;	//update the buffer
		if (i % MAX_DUMP_BYTES == MAX_DUMP_BYTES - 1) fprintf(stderr, "%s\n", buf);		//end a line
		++p;
	}
	if ((i - 1) % MAX_DUMP_BYTES != MAX_DUMP_BYTES - 1) fprintf(stderr, "%s\n", buf);
};

void stream_dump(stream_t *s) {
	fprintf(stderr, "stream length: %lu / %lu\n", s->len, s->cap);
	memory_dump(s->data, s->len);
	fprintf(stderr, "\n");
}

#endif

stream_t *stream_new(size_t size) {
	stream_t *ret = malloc(sizeof(stream_t));
	if (!ret) {
		pperror("malloc");
		return NULL;
	}
	ret->data = malloc(size);
	if (!ret->data) {
		pperror("malloc");
		free(ret);
		return NULL;
	}
	ret->len = 0;
	ret->cap = size;
	return ret;
}

void stream_free(stream_t *s) {
	if (s) {
		free(s->data);
		free(s);
	}
}

int stream_append(stream_t *s, unsigned char *data, size_t len) {
	if (s->len + len > s->cap) {
		if (stream_expand(s, len)) {
			return 1;
		}
	}
	memcpy(s->data + s->len, data, len);
	s->len += len;
	return 0;
}

int stream_expand(stream_t *s, size_t size) {
	unsigned char *new_ptr = (unsigned char *)realloc(s->data, s->cap + size);
	if (!new_ptr) {
		pperror("realloc");
		return 1;
	}
	s->cap += size;
	s->data = new_ptr;
	return 0;
}

void stream_left_shift(stream_t *s, size_t size) {
	memcpy(s->data, s->data + size, s->len - size);
	s->len -= size;
}
