#pragma once

#include <stdio.h>
#include <stdint.h>
#include <zocle/zocle.h>

typedef struct service_t
{
	char		ip[16];
	uint16_t	port;
	int			timeout;
}Service;

typedef struct name_info_t
{
	char	name[256];
	zcList	*services;
}NameInfo;

#define ACTION_UPDATE	1
#define ACTION_DEL		2

int ping_resp_pack(zcBuffer *data, int seqid);

int query_req_unpack(const char *data, int len, char *name);

int query_resp_pack(zcBuffer *data, NameInfo *info, int seqid);

int report_req_unpack(const char *data, int len, char *name, int *timestamp, int *n, int16_t *action);

int report_resp_pack(zcBuffer *data, int ret, int seqid);


int sync_req_pack(zcBuffer *data, char *auth, char *ip, char *name, int timestamp, int n, short action);

int sync_req_unpack(const char *data, int len, char *auth, char *ip, char *name, int *timestamp, int *n, short *action);

int sync_resp_pack(zcBuffer *data, int ret, int seqid);

int sync_resp_unpack(const char *data, int len, int *ret);

