
typedef unsigned char OFPMsgVersion;
typedef unsigned char OFPMsgType;
typedef unsigned short OFPMsgLength;
typedef unsigned char *OFPMsgDataPtr;

#define OFPMSG_MIN_LEN	4
#define OFPMSG_ERR_INIT	0
#define OFPMSG_ERR_LEN	1

class OFPMessage {
public:
	OFPMsgVersion version;
	OFPMsgType type;
	OFPMsgLength length;
	OFPMsgDataPtr data;
	OFPMessage(unsigned char *data, size_t len);
	~OFPMessage();
};
