#include "mmc.h"
#include <string.h>
#include "common.h"

memcached_st* openMc(const char* ip,int port)
{
	memcached_st* memc= memcached(NULL, 0); 
    if(memc)
    {   
      memcached_server_add(memc,ip,port);
    }
    return memc;

}


bool setMc(memcached_st* memc,char* key, char* value,time_t expir=0)
{
	memcached_return rc; 
	rc = memcached_set(memc,key,strlen(key),value,strlen(value),expir,0);
	if(rc == MEMCACHED_SUCCESS)
		return true;
	else
		return false;
}


/*
* here! nothing
*/
bool getMc(memcached_st* memc,char* key,char* value,int len)
{
	memcached_return rc; 
    uint32_t flags;
    size_t retLen;
	char* result = memcached_get(memc,key,strlen(key),&retLen,&flags,&rc);                                                      
    if(rc == MEMCACHED_SUCCESS)
    {   
       printf("Get value:%s\n",value);
    }   
    if(retLen>len)
    {
    	mylogF("getMc len too small");
    	return false;
    }
    strncpy(value,result,retLen);
    value[retLen]='\0';
    return true;

}

void closeMc(memcached_st* memc)
{
	if(memc!=NULL)
		memcached_free(memc);
	memc=NULL;
}