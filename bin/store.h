#pragma once

#include <stdio.h>
#include <stdint.h>
#include <zocle/zocle.h>

typedef struct store_t
{
    zcDict  *data;
}Store;

extern Store *store;

//Store*  store_create();
//int     store_save(Storage *);

int     store_load();
int     store_save();


