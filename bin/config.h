#pragma once

#include <stdio.h>
#include <stdint.h>
#include <zocle/zocle.h>

typedef struct addr_t {
    char    ip[16];
    int     port;
}Addr;

typedef struct config_t
{
	Addr	addr;
    int     timeout;
    char    homedir[256];
    char    logfile[256];
    int     loglevel;
    int8_t  daemon;
    int8_t  data_flush_mode;
    char    datafile[256];
	int		server_num;
	Addr	servers[16];
}Config;


extern Config *g_conf;

Config* config_load(char *filename);
void    config_print();

