#include "server.h"
#include "config.h"

Cluster *g_cluster;

int server_connected(zcAsynIO *conn)
{
	zcSockAddr *addr = &(conn->sock->remote);
    ZCINFO("connect to %s:%d ok", addr->ip, addr->port);
    //zc_socket_linger(conn->sock, 1, 0); 
    //zc_asynio_read_bytes(conn, 4, conn_read_head);
    //ZCDEBUG("_read_bytes:%d %p", conn->_read_bytes, conn);

    return ZC_OK;
}


int
server_connect_fail(zcAsynIO *conn, int err)
{
    ZCNOTE("connection error! %d", err);
    return ZC_OK;
}

Cluster*    
cluster_create()
{
    Cluster *clu = zc_malloct(Cluster);
    int i;
    zcAsynIO    *conn;
    Addr        *addr;

    zcProtocol p;
    zc_protocol_init(&p);
	p.handle_error = server_connect_fail;
    p.handle_connected = server_connected;

    for (i=0; i<g_conf->server_num; i++) {
        addr = &g_conf->servers[i];
        conn = zc_asynio_new_tcp_client(addr->ip, addr->port, g_conf->timeout, &p, g_loop, 65535, 65535);
        clu->server[i] = conn;
    }

    g_cluster = clu;
	return clu;
}

