#pragma once
#include <libmemcached/memcached.h>
extern memcached_st* openMc(const char* ip,int port);
extern bool setMc(memcached_st* memc,char* key, char* value,time_t expir=0);
extern bool getMc(memcached_st* memc,char* key,char* value,int Len);
extern void closeMc(memcached_st* );