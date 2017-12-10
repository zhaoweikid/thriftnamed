#include "config.h"

Config *g_conf;

Config* 
config_load(char *filename)
{
    g_conf = zc_calloct(Config);

    strcpy(g_conf->ip, "127.0.0.1");
    g_conf->port = 11111;
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
    snprintf(g_conf->ip, sizeof(g_conf->ip), "%s", value);
    g_conf->port = zc_confdict_get_int(cfdic, "main", "port", 0); 
    if (g_conf->port == 0) {
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

	return g_conf;
}

void    
config_print()
{
}


