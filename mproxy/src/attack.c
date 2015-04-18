#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "global.h"
#include "attack.h"
#include "stream.h"
#include "csv.h"
#include "args.h"

char atk_drop_action_table[OFP_VER_MAX][OFP_MSG_TYPE_MAX + 1];

static char *OFP_V1_0_MSG_ALIAS[] = {
	"Hello", "Error", "EchoReq", "EchoRes", "Vendor",
	"FeatureReq", "FeatureRes", "GetConfigReq", "GetConfigRes", "SetConfig",
	"PacketIn", "FlowRemoved", "PortStatus", "PacketOut", "FlowMod", 
	"PortMod", "StatsReq", "StatsRes", "BarrierReq", "BarrierRes", 
	"QueueGetConfigReq", "QueueGetConfigRes"
};

static char *OFP_V1_1_MSG_ALIAS[] = {
	"Hello", "Error", "EchoReq", "EchoRes", "Experimenter",
	"FeatureReq", "FeatureRes", "GetConfigReq", "GetConfigRes", "SetConfig",
	"PacketIn", "FlowRemoved", "PortStatus", "PacketOut", "FlowMod", 
	"GroupMod", "PortMod", "TableMod", "StatsReq", "StatsRes", 
	"BarrierReq", "BarrierRes", "QueueGetConfigReq", "QueueGetConfigRes"
};

static char *OFP_V1_2_MSG_ALIAS[] = {
	"Hello", "Error", "EchoReq", "EchoRes", "Experimenter",
	"FeatureReq", "FeatureRes", "GetConfigReq", "GetConfigRes", "SetConfig",
	"PacketIn", "FlowRemoved", "PortStatus", "PacketOut", "FlowMod", 
	"GroupMod", "PortMod", "TableMod", "StatsReq", "StatsRes", 
	"BarrierReq", "BarrierRes", "QueueGetConfigReq", "QueueGetConfigRes", "RoleReq",
	"RoleRes"
};

static char *OFP_V1_3_MSG_ALIAS[] = {
	"Hello", "Error", "EchoReq", "EchoRes", "Experimenter",
	"FeatureReq", "FeatureRes", "GetConfigReq", "GetConfigRes", "SetConfig",
	"PacketIn", "FlowRemoved", "PortStatus", "PacketOut", "FlowMod", 
	"GroupMod", "PortMod", "TableMod", "MultipartReq", "MultipartRes"
	"BarrierReq", "BarrierRes", "QueueGetConfigReq", "QueueGetConfigRes", "RoleReq",
	"RoleRes", "GetAsyncReq", "GetAsyncRes", "SetAsync", "MeterMod"
};

static char *OFP_V1_4_MSG_ALIAS[] = {
	"Hello", "Error", "EchoReq", "EchoRes", "Experimenter",
	"FeatureReq", "FeatureRes", "GetConfigReq", "GetConfigRes", "SetConfig",
	"PacketIn", "FlowRemoved", "PortStatus", "PacketOut", "FlowMod", 
	"GroupMod", "PortMod", "TableMod", "MultipartReq", "MultipartRes"
	"BarrierReq", "BarrierRes", NULL, NULL, "RoleReq",
	"RoleRes", "GetAsyncReq", "GetAsyncRes", "SetAsync", "MeterMod",
	"RoleStatus", "TableStatus", "RequestForward", "BundleControl", "BundleAddMessage"
};

static char *OFP_COMMON_MSG_ALIAS[] = {
	"Hello", "Error", "EchoReq", "EchoRes", NULL,
	"FeatureReq", "FeatureRes", "GetConfigReq", "GetConfigRes", "SetConfig",
	"PacketIn", "FlowRemoved", "PortStatus", "PacketOut", "FlowMod",
	NULL, NULL, NULL, NULL, NULL, 
	NULL, NULL, NULL, NULL, "RoleReq", 
	"RoleRes", "GetAsyncReq", "GetAsyncRes", "SetAsync", "MeterMod",
	"RoleStatus", "TableStatus", "RequestForward", "BundleControl", "BundleAddMessage"
};

static int OFP_MSG_ALIAS_LEN[OFP_VER_MAX + 1] = {35, 22, 24, 26, 30, 35};

static char **OFP_MSG_ALIASES[] = {
	OFP_COMMON_MSG_ALIAS, 
	OFP_V1_0_MSG_ALIAS, 
	OFP_V1_1_MSG_ALIAS, 
	OFP_V1_2_MSG_ALIAS, 
	OFP_V1_3_MSG_ALIAS, 
	OFP_V1_4_MSG_ALIAS
};

int normalize_ofp_ver_str(char *v) {
	int ret;

	if (strchr(v, '.')) {
		// string of format x.y or x.y.z
		if (!strncmp(v, "1.0", 3)) return OFP_VER_1_0;
		if (!strncmp(v, "1.1", 3)) return OFP_VER_1_1;
		if (!strncmp(v, "1.2", 3)) return OFP_VER_1_2;
		if (!strncmp(v, "1.3", 3)) return OFP_VER_1_3;
		if (!strncmp(v, "1.4", 3)) return OFP_VER_1_4;
	}
	
	// wildcard expression
	if (v[0] == '*') return OFP_VER_ALL;

	// the last possibility is that v is the version value, not string.
	ret = atoi(v);
	if (ret < OFP_VER_ALL || ret > OFP_VER_MAX) return OFP_VER_ERR;
	return ret;
}

int normalize_ofp_msg_type(int ver, char *s) {
	int ret, i;
	int len = OFP_MSG_ALIAS_LEN[ver];
	char *alias;

	if (s[0] == '*') return OFP_MSG_TYPE_ALL;
	if (is_int(s)) {
		ret = atoi(s);
		if (ret > len || ret < 0 || OFP_MSG_ALIASES[ver][ret] == NULL)
			return OFP_MSG_TYPE_ERR;
		return ret;
	}

	for (i = 0; i < len; ++i) {
		alias = OFP_MSG_ALIASES[ver][i];
		if (alias && !strcmp(alias, s)) {
			return i;
		}
	}

	// alias not found
	return OFP_MSG_TYPE_ERR;
}

int normalize_action_type(char *s) {
	int ret;
	if (is_int(s)) {
		ret = atoi(s);
		if (ret < ACTION_ID_MIN || ret > ACTION_ID_MAX) return ACTION_ID_ERR;
		return ret;
	}
	if (!strcmp(ACTION_ALIAS_DROP, s)) return ACTION_ID_DROP;
	if (!strcmp(ACTION_ALIAS_DELAY, s)) return ACTION_ID_DELAY;
	if (!strcmp(ACTION_ALIAS_DUP, s)) return ACTION_ID_DUP;
	if (!strcmp(ACTION_ALIAS_LIE, s)) return ACTION_ID_LIE;
	return ACTION_ID_ERR;
}

int attacker_init() {
	// initialize random seed
	srand(time(NULL));
	return 0;
}

void attacker_read_rows(char *fpath) {
	int c, line_no = 1;
	char ch;
	FILE *f;
	stream_t *stream;

	f = fopen(fpath, "r");
	if (!f) {
		perror("fopen");
		return;
	}

	stream = stream_new(DEFAULT_STREAM_LEN);
	if (!stream) {
		err(COLOR_RED "attacker_read_rows: cannot alloc data stream.\n" COLOR_BLACK);
		return;
	}

	while (!feof(f)) {
		while ((c = fgetc(f)) != '\n' && c != '\r' && c != EOF) {
			ch = c;
			stream_append(stream, (unsigned char *)&ch, 1);
		}
		ch = '\0';
		stream_append(stream, (unsigned char *)&ch, 1);
		// stream_dump(stream);
		if (line_no > 1 && stream->len > 1 && stream->data[0] != '#') {
			attacker_add_rule(line_no, (char *)stream->data, stream->len);
		}
		++line_no;
		stream->len = 0;
	}

	stream_free(stream);
	fclose(f);
}

int attacker_add_rule(int id, char *data, size_t len) {
	size_t num_fields;
	//ssize_t row_len;
	//int cid, sid;
	int i, j;
	int ofp_ver;
	int msg_type;
	int action_type;
	char **fields;
	arg_node_t *args, *targ;

	fields = csv_parse(data, len, &num_fields);
	/*#ifdef _DEBUG
	for (i = 0; i < num_fields; ++i) {
		//csv_unescape(fields[i]);
		fprintf(stderr, "%d: \"%s\"\n", i, fields[i]);
	}
	#endif*/
	if (num_fields != ATTACKER_ROW_NUM_FIELDS) {
		err(COLOR_RED "Attack rule %d: csv field count mismatch (%lu / %d).\n" COLOR_BLACK, id, num_fields, ATTACKER_ROW_NUM_FIELDS);
		csv_free(fields);
		return 1;
	}
	//row_len = atoi(fields[0]);
	//cid = atoi(fields[1]);
	//sid = atoi(fields[2]);
	if ((ofp_ver = normalize_ofp_ver_str(fields[3])) == OFP_VER_ERR) {
		err(COLOR_RED "Attack rule %d: unrecognized OFP version field \"%s\".\n" COLOR_BLACK, id, fields[3]);
		csv_free(fields);
		return 1;
	}
	if ((msg_type = normalize_ofp_msg_type(ofp_ver, fields[4])) == OFP_MSG_TYPE_ERR) {
		err(COLOR_RED "Attack rule %d: unrecognized message type \"%s\" for OFP version %d.\n" COLOR_BLACK, id, fields[4], ofp_ver);
		csv_free(fields);
		return 1;
	}
	// TODO: parse field column
	if ((action_type = normalize_action_type(fields[6])) == ACTION_ID_ERR) {
		err(COLOR_RED "Attack rule %d: unsupported malicious action \"%s\".\n" COLOR_BLACK, id, fields[6]);
		csv_free(fields);
		return 1;
	}

	if (action_type == ACTION_ID_DROP) {
		char prob = 100;
		if ((args = args_parse(fields[7], ATTACKER_ARGS_DELIM))) {
			if ((targ = args_find(args, "p"))) {
				if (targ->type == ARG_VALUE_TYPE_INT)
					prob = targ->value.i;
				else if (targ->type == ARG_VALUE_TYPE_FLOAT)
					prob = (char)targ->value.f;
			}
			if (prob < 0) prob = 0;
			else if (prob > 100) prob = 100;
			args_free(args);
		}
		if (ofp_ver == OFP_VER_ALL) {
			for (i = 0; i < OFP_VER_MAX; ++i) {
				if (msg_type == OFP_MSG_TYPE_ALL) {
					for (j = 0; j <= OFP_MSG_TYPE_MAX; ++j) {
						atk_drop_action_table[i][j] = prob;
					}
				} else {
					atk_drop_action_table[i][msg_type] = prob;
				}
			}
		} else {
			if (msg_type == OFP_MSG_TYPE_ALL) {
				for (j = 0; j <= OFP_MSG_TYPE_MAX; ++j) {
					atk_drop_action_table[ofp_ver - 1][j] = prob;
				}
			} else {
				atk_drop_action_table[ofp_ver - 1][msg_type] = prob;
			}
		}
		log(COLOR_GREEN "Attack rule %d: accepted DROP rule (probability %d%%).\n" COLOR_BLACK, id, prob);
		#ifdef _DEBUG
		for (i = 0; i < OFP_VER_MAX; ++i) {
			for (j = 0; j <= OFP_MSG_TYPE_MAX; ++j) {
				fprintf(stderr, "%d ", atk_drop_action_table[i][j]);
			}
			fputc('\n', stderr);
		}
		#endif
	} else {
		err(COLOR_YELLOW "Attack rule %d: unimplemented attack type \"%s\" (%d).\n" COLOR_BLACK, id, fields[6], action_type);
	}

	csv_free(fields);
	return 0;
}
