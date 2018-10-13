#include "network.h"

int ping_resp_pack(zcBuffer *data, int seqid)
{
	zc_thrift_write_framed_head(data);
	zc_thrift_write_msg_begin(data, "ping", ZC_THRIFT_REPLY, seqid);

	zc_thrift_write_field_stop(data);
	zc_thrift_write_framed(data);

	return ZC_OK;
}


int query_req_unpack(const char *data, int len, char *name)
{
	int		i = 0;
	char	type = 0;
	int16_t	id = 0;

	i = zc_thrift_read_field_begin(data, NULL, &type, &id);
	if (type != ZC_THRIFT_STRING) {
		return ZC_ERR;
	}
	int namelen = 0;
	//i = zc_thrift_read_i32(data+i, &namelen);
	zc_thrift_read_binary(data+i, name, &namelen);

	return ZC_OK;
}


int query_resp_pack(zcBuffer *data, NameInfo *info, int seqid)
{
	zc_thrift_write_framed_head(data);
	zc_thrift_write_msg_begin(data, "query", ZC_THRIFT_REPLY, seqid);

	zc_thrift_write_struct_begin(data, NULL);
	zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_STRING, 1);
	zc_thrift_write_binary(data, info->name, strlen(info->name));

	zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_LIST, 2);
	zc_thrift_write_list_begin(data, ZC_THRIFT_STRUCT, info->services->size); 

	zcListNode *node;
	zc_list_foreach(info->services, node) {
		Service *serv = (Service*)node->data;

		zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_STRING, 1);
		zc_thrift_write_binary(data, serv->ip, strlen(serv->ip));

		zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_I16, 2);
		zc_thrift_write_i16(data, serv->port);

		zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_I32, 3);
		zc_thrift_write_i32(data, serv->timeout);

		zc_thrift_write_struct_end(data);
	}

	zc_thrift_write_struct_end(data);
	zc_thrift_write_msg_end(data);

	zc_thrift_write_framed(data);

	return ZC_OK;
}


int report_req_unpack(const char *data, int len, char *name, int *timestamp, int *n)
{
	int		i = 0;
	char	type = 0;
	int16_t	id = 0;
	int		namelen = 0;

	i = zc_thrift_read_field_begin(data, NULL, &type, &id);
	if (type != ZC_THRIFT_STRING) {
		return ZC_ERR;
	}
	//i = zc_thrift_read_binary(data+i, &len);
	i = zc_thrift_read_binary(data+i, name, &namelen);

	i = zc_thrift_read_field_begin(data+i, NULL, &type, &id);
	if (type != ZC_THRIFT_I32) {
		return ZC_ERR;
	}
	i = zc_thrift_read_i32(data+i, timestamp);

	i = zc_thrift_read_field_begin(data+i, NULL, &type, &id);
	if (type != ZC_THRIFT_I32) {
		return ZC_ERR;
	}
	i = zc_thrift_read_i32(data+i, n);

	return ZC_OK;
}


int report_resp_pack(zcBuffer *data, int ret, int seqid)
{
	zc_thrift_write_framed_head(data);
	zc_thrift_write_msg_begin(data, "report", ZC_THRIFT_REPLY, seqid);

	zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_I32, 1);
	zc_thrift_write_i32(data, ret);

	zc_thrift_write_msg_end(data);
	zc_thrift_write_framed(data);

	return ZC_OK;
}


int sync_req_pack(zcBuffer *data, char *auth, char *ip, char *name, int timestamp, int n)
{
	//srandomdev();
	srandom((unsigned int)time(NULL));
	int seqid = random() % 2000000000;

	zc_thrift_write_framed_head(data);
	zc_thrift_write_msg_begin(data, "sync", ZC_THRIFT_CALL, seqid);

	zc_thrift_write_field_begin(data, "auth", ZC_THRIFT_STRING, 1);
	zc_thrift_write_binary(data, auth, strlen(auth));

	zc_thrift_write_field_begin(data, "ip", ZC_THRIFT_STRING, 2);
	zc_thrift_write_binary(data, ip, strlen(ip));

	zc_thrift_write_field_begin(data, "name", ZC_THRIFT_STRING, 3);
	zc_thrift_write_binary(data, name, strlen(name));

	zc_thrift_write_field_begin(data, "timestamp", ZC_THRIFT_I32, 4);
	zc_thrift_write_i32(data, timestamp);

	zc_thrift_write_field_begin(data, "n", ZC_THRIFT_I32, 5);
	zc_thrift_write_i32(data, n);

	zc_thrift_write_msg_end(data);
	zc_thrift_write_framed(data);

	return ZC_OK;
}


int sync_req_unpack(const char *data, int len, char *auth, char *ip, char *name, int *timestamp, int *n)
{
	int i;
	int16_t id;
	char	type;
	int32_t	slen;

	i = zc_thrift_read_field_begin(data, NULL, &type, &id);
	if (type != ZC_THRIFT_STRING) {
		return ZC_ERR;
	}
	zc_thrift_read_binary(&data[i], auth, &slen);
	auth[slen] = 0;

	i = zc_thrift_read_field_begin(&data[i], NULL, &type, &id);
	if (type != ZC_THRIFT_STRING) {
		return ZC_ERR;
	}
	i += zc_thrift_read_binary(&data[i], ip, &slen);
	ip[slen] = 0;

	i += zc_thrift_read_field_begin(&data[i], NULL, &type, &id);
	if (type != ZC_THRIFT_STRING) {
		return ZC_ERR;
	}
	i += zc_thrift_read_binary(&data[i], name, &slen);
	name[slen] = 0;

	i += zc_thrift_read_field_begin(&data[i], NULL, &type, &id);
	if (type != ZC_THRIFT_I32) {
		return ZC_ERR;
	}
	i += zc_thrift_read_i32(&data[i], timestamp);

	i += zc_thrift_read_field_begin(&data[i], NULL, &type, &id);
	if (type != ZC_THRIFT_I32) {
		return ZC_ERR;
	}
	i += zc_thrift_read_i32(&data[i], n);

	i += zc_thrift_read_field_begin(&data[i], NULL, &type, &id);
	if (type != ZC_THRIFT_STOP) {
		return ZC_ERR;
	}

	
	return ZC_OK;
}

int sync_resp_pack(zcBuffer *data, int ret, int seqid)
{
	zc_thrift_write_framed_head(data);
	zc_thrift_write_msg_begin(data, "report", ZC_THRIFT_REPLY, seqid);

	zc_thrift_write_field_begin(data, NULL, ZC_THRIFT_I32, 1);
	zc_thrift_write_i32(data, ret);

	zc_thrift_write_msg_end(data);
	zc_thrift_write_framed(data);

	return ZC_OK;
}

int sync_resp_unpack(const char *data, int len, int *ret)
{
	int		i = 0;
	char	type;
	int16_t	id;
	i = zc_thrift_read_field_begin(data, NULL, &type, &id);
	if (type != ZC_THRIFT_I32) {
		return ZC_ERR;
	}

	zc_thrift_read_i32(&data[i], ret);
	
	return ZC_OK;
}







