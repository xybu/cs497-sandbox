#include <cstdlib>
#include <cstring>
#include "ofp.h"

OFPMessage::OFPMessage(unsigned char *raw, size_t len) {
	if (!raw || len < OFPMSG_MIN_LEN) 
		throw OFPMSG_ERR_INIT;
	version = *((OFPMsgVersion *)raw);
	type = *((OFPMsgType *)(raw + sizeof(OFPMsgVersion)));
	length = *((OFPMsgLength *)(raw + sizeof(OFPMsgVersion) + sizeof(OFPMsgType)));
	if (length == 0) {
		data = NULL;
	} else if (length > len - OFPMSG_MIN_LEN) {
		data = (unsigned char *)malloc(length);
		if (!data) throw OFPMSG_ERR_INIT;
		memcpy(data, raw, length);
	} else {
		throw OFPMSG_ERR_LEN;
	}
}

OFPMessage::~OFPMessage() {
	if (data) free(data);
}
