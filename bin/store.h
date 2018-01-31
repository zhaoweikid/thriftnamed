#pragma once

#include <stdio.h>
#include <stdint.h>
#include <zocle/zocle.h>

typedef struct service_t
{
    char    ip[16];
    int16_t port;
    int32_t timeout;
    int32_t weight;
    int32_t load;
}Service;

typedef struct store_t
{
    zcDict  *data;
}Store;

extern Store *store;

//Store*  store_create();
//int     store_save(Storage *);

int     store_load();
int     store_save();


