/**
 * stream.cpp
 * A simple byte stream wrapper.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#include <cstdio>
#include <cstring>
#include <exception>
#include "global.h"
#include "stream.h"

#define MAX_DUMP_BYTES 16
#define SMALLEST_VASCII	32
#define LARGEST_VASCII	126

static void memory_dump(unsigned char *p, int len) {
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

Stream::Stream(size_t size) {
	data = (unsigned char *)malloc(size);
	if (!data) {
		perror("malloc");
		throw 0;
	}
	len = 0;
	capacity = size;
}

Stream::~Stream() {
	free(data);
	data = NULL;
	len = 0;
	capacity = 0;
}

void Stream::append(void *src, size_t size) {
	if (len + size > capacity) {
		expand(size);
	}
	memcpy(data + len, src, size);
	len += size;
}

void Stream::append2(Stream *stream) {
	append(stream->data, stream->len);
}

void Stream::expand(size_t size) {
	unsigned char *new_ptr = (unsigned char *)realloc(data, capacity + size);
	if (!new_ptr) {
		perror("realloc");
		throw 0;
	}
	capacity += size;
	data = new_ptr;
}

void Stream::dump() {
	fprintf(stderr, "stream length: %lu / %lu\n", len, capacity);
	memory_dump(data, len);
	fprintf(stderr, "\n");
}
