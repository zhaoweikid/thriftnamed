#include "config.h"

Config *g_conf;


static int 
parse_addr(char *s, char sp, Addr *addr)
{
	char *p = s;
	int addri = 0;
	while (*p) {
		Addr *cur = &addr[addri];
		// skip blank
		while (*p && (isblank(*p) || (*p == '\n') || (*p == '\r'))) p++;

		// parse ip
		char *p1 = strchr(p, ':');	
		if (NULL == p1)
			return ZC_ERR;
		
		if ((p1 - p) > 15)
			return ZC_ERR_RANGE;
		
		memset(cur->ip, 0, sizeof(cur->ip));
		strncpy(cur->ip, p, p1-p);

		p = p1 + 1; // skip :

		// parse port
		char tmp[8] = {0};
		int i = 0;
		while ((*p != sp && *p ) && !isblank(*p)) {
			tmp[i] = *p;
			p++;	
			i++;
		}
		if (i >= 6) // port must < 65535
			return ZC_ERR;	

		cur->port = atoi(tmp);
		if (cur->port > 65535) 
			return ZC_ERR;
	
		while (isblank(*p) || (*p == sp)) p++;	
		addri++;
	}

	return addri;
}


Config* 
config_load(char *filename)
{
    g_conf = zc_calloct(Config);

    strcpy(g_conf->addr.ip, "127.0.0.1");
    g_conf->addr.port = 11111;
    g_conf->timeout = 1000; // ms
    
    zcConfDict *cfdic = zc_confdict_new(filename);
    if (NULL == cfdic) {
        ZCFATAL("config file error: %s\n", filename);
    }   

    int ret = zc_confdict_parse(cfdic);
    if (ret != 0) {
        ZCFATAL("config file %s parse error: %d\n", filename, ret);
    }   
    
    //ZCDEBUG("====================\n"); 
    char *value = NULL; 

    value = zc_confdict_get_str(cfdic, "main", "ip", NULL);
    if (NULL == value) {
        ZCFATAL("main.ip parse error!");
    }   
    snprintf(g_conf->addr.ip, sizeof(g_conf->addr.ip), "%s", value);

    g_conf->addr.port = zc_confdict_get_int(cfdic, "main", "port", 0); 
    if (g_conf->addr.port == 0) {
        ZCFATAL("main.port parse error!");
    }   
    
    g_conf->timeout = zc_confdict_get_int(cfdic, "main", "timeout", -1);
    if (g_conf->timeout == -1) {
        ZCFATAL("main.timeout parse error!");
    }

    g_conf->daemon = zc_confdict_get_int(cfdic, "main", "daemon", 0);
    if (g_conf->daemon != 0 && g_conf->daemon != 1) {
        ZCFATAL("main.daemon parse error!");
    }

	g_conf->data_flush_mode = zc_confdict_get_int(cfdic, "main", "data_flush_mode", -2);
    if (g_conf->data_flush_mode == -2) {
        ZCFATAL("main.data_flush_mode parse error!");
    }

    value = zc_confdict_get_str(cfdic, "main", "homedir", NULL);
    if (NULL == value) {
        ZCFATAL("main.homedir parse error!");
    }
    snprintf(g_conf->homedir, sizeof(g_conf->homedir), "%s", value);

    value = zc_confdict_get_str(cfdic, "main", "logfile", NULL);
    if (NULL == value) {
        ZCFATAL("main.logfile parse error!");
    }
    if (*value == '/') {
        snprintf(g_conf->logfile, sizeof(g_conf->logfile), "%s", value);
    }else if (strcmp(value, "stdout") != 0){
        snprintf(g_conf->logfile, sizeof(g_conf->logfile), "%s/%s", g_conf->homedir, value);
    }else{
        strcpy(g_conf->logfile, value);
    }

    value = zc_confdict_get_str(cfdic, "main", "loglevel", NULL);
    if (NULL == value) {
        ZCFATAL("main.loglevel parse error!");
    }
	
	if (strcmp(value, "INFO") == 0 || strcmp(value, "DEBUG") == 0) {
        g_conf->loglevel = ZC_LOG_DEBUG;
    }else if (strcmp(value, "NOTICE") == 0) {
        g_conf->loglevel = ZC_LOG_NOTICE;
    }else if (strcmp(value, "WARN") == 0) {
        g_conf->loglevel = ZC_LOG_WARN;
    }else if (strcmp(value, "ERROR") == 0) {
        g_conf->loglevel = ZC_LOG_ERROR;
    }else{
        ZCFATAL("loglevel error: %s", value);
    }

    value = zc_confdict_get_str(cfdic, "main", "datafile", NULL);
    if (NULL == value) {
        ZCFATAL("main.datafile parse error!");
    }
    if (*value == '/') {
        snprintf(g_conf->datafile, sizeof(g_conf->datafile), "%s", value);
    }else{
        snprintf(g_conf->datafile, sizeof(g_conf->datafile), "%s/%s", g_conf->homedir, value);
    }


    value = zc_confdict_get_str(cfdic, "main", "servers", NULL);
    if (NULL == value) {
        ZCFATAL("main.server parse error!");
    }
	
	g_conf->server_num = parse_addr(value, ',', g_conf->servers);
	if (g_conf->server_num <= 0) {
        ZCFATAL("main.server parse error!");
	}
		

	return g_conf;
}

void    
config_print()
{
	int i = 0;

	printf("addr.ip\t\t%s\n", g_conf->addr.ip);
	printf("addr.port\t%d\n", g_conf->addr.port);
	printf("timeout\t\t%d\n", g_conf->timeout);
	printf("server_num\t%d\n", g_conf->server_num);

	for (i=0; i<g_conf->server_num; i++) {
		printf("servers\t\t%s:%d\n", g_conf->servers[i].ip, g_conf->servers[i].port);
	}


}


