#include <zocle/zocle.h>
#include "config.h"
#include "network.h"
#include "server.h"
#include "store.h"

struct ev_loop *g_loop;

int conn_read_head(zcAsynIO *conn, const char *data, int len);

int conn_read_body(zcAsynIO *conn, const char *data, int len)
{
	char tmp[1000];

	zc_format_hex(tmp, data, len);

	int i;

    char name[256] = {0};
	char type = 0;
	int  seqid = 0;
	
	i = zc_thrift_read_msg_begin(data, name, &type, &seqid);
	ZCDEBUG("name:%s type:%d seqid:%d", name, type, seqid);
	
	zc_buffer_compact(conn->wbuf);

	if (strcmp(name, "ping") == 0) {
		ping_resp_pack(conn->wbuf, seqid);
	}else if (strcmp(name, "query") == 0) {
		char service_name[256] = {0};
		query_req_unpack(&data[i], len-i, service_name);
		ZCDEBUG("service name:%s", service_name);
	
		NameInfo *nameinfo = zc_malloct(NameInfo);
		strcpy(nameinfo->name, service_name);
		nameinfo->services = zc_list_new();
		
		Service *s1 = zc_malloct(Service);
		strcpy(s1->ip, "127.0.0.1");
		s1->port = 10001;
		s1->timeout = 1000;
		zc_list_append(nameinfo->services, s1);

		Service *s2 = zc_malloct(Service);
		strcpy(s2->ip, "127.0.0.2");
		s2->port = 10002;
		s2->timeout = 1000;
		zc_list_append(nameinfo->services, s2);

		Service *s3 = zc_malloct(Service);
		strcpy(s3->ip, "127.0.0.3");
		s3->port = 10003;
		s3->timeout = 1000;
		zc_list_append(nameinfo->services, s3);

		ZCDEBUG("pack resp");
		query_resp_pack(conn->wbuf, nameinfo, seqid);
	}else if (strcmp(name, "report") == 0) {
		//zc_thrift_write_exception(conn->wbuf, "not found function name", ZC_THRIFT_ERR_WRONG_MSG_NAME, name, seqid, true);
	
		char service_name[256] = {0};
		int timestamp;
		int16_t action;
		int n;
		report_req_unpack(&data[i], len-i, service_name, &timestamp, &n, &action);

		ZCDEBUG("name:%s timestamp:%d n:%d action:%d", service_name, timestamp, n, action);

	}else if (strcmp(name, "sync") == 0) {
		zc_thrift_write_exception(conn->wbuf, "not found function name", ZC_THRIFT_ERR_WRONG_MSG_NAME, name, seqid, true);
	}else if (strcmp(name, "get") == 0) {
		zc_thrift_write_exception(conn->wbuf, "not found function name", ZC_THRIFT_ERR_WRONG_MSG_NAME, name, seqid, true);
	}else{

		zc_thrift_write_exception(conn->wbuf, "not found function name", ZC_THRIFT_ERR_WRONG_MSG_NAME, name, seqid, true);
	}
	
	zc_format_hex(tmp, zc_buffer_data(conn->wbuf), zc_buffer_used(conn->wbuf));

	ZCDEBUG("reply data: [%d] %s\n", zc_buffer_used(conn->wbuf), tmp);
	ZCDEBUG("------------------------------------");

	zc_asynio_read_bytes(conn, 4, conn_read_head);

    return len;
}


int conn_read_head(zcAsynIO *conn, const char *data, int len)
{
	int headlen = 0;

	memcpy(&headlen, data, 4); 
	headlen = zc_htob32(headlen);
    ZCDEBUG("read body:%d", headlen);
	zc_asynio_read_bytes(conn, headlen, conn_read_body);

	return ZC_OK;
}


int conn_connected(zcAsynIO *conn)
{
	ZCINFO("connected!");
	zc_socket_linger(conn->sock, 1, 0); 
	zc_asynio_read_bytes(conn, 4, conn_read_head);
    //ZCDEBUG("_read_bytes:%d %p", conn->_read_bytes, conn);

	return ZC_OK;
}


int main(int argc, char * argv[]) 
{
    zc_mem_init(ZC_MEM_GLIBC|ZC_MEM_DBG_OVERFLOW);

    if (argc != 2) {
        printf("usage:\n\tthriftnamed configfile\n\n");
        return -1;
    }

	char *filename = argv[1];
    config_load(filename);
    config_print();

    zc_log_new(g_conf->logfile, g_conf->loglevel);

	g_loop = ev_default_loop (0);	

	zcProtocol p;
	zc_protocol_init(&p);
	p.handle_connected = conn_connected;

	ZCNOTICE("thriftnamed start at %s:%d\n", g_conf->addr.ip, g_conf->addr.port);
	zcAsynIO *server = zc_asynio_new_tcp_server(g_conf->addr.ip, g_conf->addr.port, 
                g_conf->timeout, &p, g_loop, 32768, 32768);
	if (NULL == server) {
		ZCERROR("server create error!");
		return -1;
	}

    cluster_create();

	ev_run (g_loop, 0); 
	ZCINFO("stopped");

    return 0;
}
