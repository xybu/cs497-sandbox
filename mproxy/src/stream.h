/**
 * stream.h
 * Class declaration for a simple byte stream wrapper.
 * 
 * @author	Xiangyu Bu <xb@purdue.edu>
 */

#ifndef _STREAM_H

#define _STREAM_H

#include <cstdlib>

#define DEFAULT_STREAM_LEN	512

class Stream {
public:
	unsigned char *data;
	size_t capacity;
	size_t len;

	Stream(size_t size = DEFAULT_STREAM_LEN);
	~Stream();
	void append(void *src, size_t size);
	void append2(Stream *stream);
	void expand(size_t size);
	void dump();
};

#endif
