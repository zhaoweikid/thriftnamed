#pragma once

#include <zocle/zocle.h>

typedef struct server_t
{
    zcSocket    *conn;
    int64_t     seqno;
    int32_t     lasttime;
}Server;

typedef struct cluster_t
{
    zcAsynIO    *server[64];
}Cluster;

extern struct ev_loop *g_loop;

extern Cluster *g_cluster;

Cluster*    cluster_create();



