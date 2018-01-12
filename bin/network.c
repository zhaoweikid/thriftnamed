#include "network.h"

int ping_pack_resp(zcBuffer *data, char *name, int seqid)
{
	zc_thrift_write_framed_head(data);
	zc_thrift_write_msg_begin(data, name, ZC_THRIFT_REPLY, seqid);

	zc_thrift_write_field_stop(data);
	zc_thrift_write_framed(data);

	return 0;
}



