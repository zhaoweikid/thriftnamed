#include <zocle/zocle.h>
#include "config.h"
#include "endian_swap.h"

int conn_read_body(zcAsynIO *conn, const char *data, int len)
{
    int namelen = 0;

    memcpy(&namelen, data+4, 4); 
    namelen = htob32(namelen);
    ZCDEBUG("namelen:%d", namelen);

    char name[256] = {0};
    strncpy(name, data+8, namelen);

    ZCDEBUG("name:%s", name);
	ZCDEBUG("data:%s", data);

    return ZC_OK;
}


int conn_read_head(zcAsynIO *conn, const char *data, int len)
{
	int headlen = 0;

	memcpy(&headlen, data, 4); 
	headlen = htob32(headlen);
    ZCDEBUG("read body:%d", headlen);
	zc_asynio_read_bytes(conn, headlen, conn_read_body);

	return ZC_OK;
}


int conn_connected(zcAsynIO *conn)
{
	ZCINFO("connected!");
	zc_socket_linger(conn->sock, 1, 0); 
	zc_asynio_read_bytes(conn, 4, conn_read_head);
    ZCDEBUG("_read_bytes:%d %p", conn->_read_bytes, conn);

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

	struct ev_loop *loop = ev_default_loop (0);	

	zcProtocol p;
	zc_protocol_init(&p);
	p.handle_connected = conn_connected;

	ZCNOTICE("thriftnamed start at %s:%d\n", g_conf->addr.ip, g_conf->addr.port);
	zcAsynIO *server = zc_asynio_new_tcp_server(g_conf->addr.ip, g_conf->addr.port, 
                g_conf->timeout, &p, loop, 32768, 32768);
	if (NULL == server) {
		ZCERROR("server create error!");
		return -1;
	}

	ev_run (loop, 0); 
	ZCINFO("stopped");

    return 0;
}
