#include<stdlib.h>
#include "util.h"
#include<sys/time.h>
#include <openssl/md5.h>

const char* RANDTABLE="abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@_<#%->^*i+"; 
const int RANDOM_STRING_MAX = 1024;
const int TABLELEN = 71;
 
std::string getMd5(const std::string& in)
{
    MD5_CTX hash_ctx;
    unsigned char hash_ret[16];        
    std::string out;
    char tmp[3]={0};
    MD5_Init(&hash_ctx);
    MD5_Update(&hash_ctx,in.c_str(),in.length());
    MD5_Final(hash_ret, &hash_ctx);
    for(int i=0; i<16; i++ ){
        snprintf(tmp,3,"%02x",hash_ret[i]);
        out+=tmp;
    } 

    return out;
}


int getRandInt(int low,int high)
{
	int ret; 
	struct timeval nowtime;
	gettimeofday(&nowtime, NULL);
	srand((nowtime.tv_sec*1000000+nowtime.tv_usec));
	if(low==high)
		ret=low;
	else                     
		ret=rand()%(high-low)+low;
	return ret;
}



std::string getRandString(int low,int high)
{
	if(low<0)
        low=0;
    if(high > RANDOM_STRING_MAX)
        high=RANDOM_STRING_MAX;     
    if(low>high)
    {
        int tmp=low;
        low=high;
        high=tmp;
    }     
    char str[RANDOM_STRING_MAX]={'\0'};
    
    struct timeval nowtime;
    gettimeofday(&nowtime, NULL);
    srand((nowtime.tv_sec*1000000+nowtime.tv_usec));   
    
    int size=getRandInt(low,high);
    str[0]='"';
    for(int i=1;i!=size+1;++i)
    {   
        str[i]=RANDTABLE[rand()%TABLELEN];
    }   
    str[size+1]='"';
    std::string ret(str);
    return ret;
}


void printDset(const DSET& container)
{
    
    mylogD("====================print dset======================================");
    for(DynamicInfo each : container)
    {
        mylogS(" %d_%d ",each.pid,each.pageIndex);
    }
    mylogD("\n====================================================================");
    
}

void printPset(const PSET& container, int pi)
{

    mylogD("====================print pset======================================");
    for(uint32_t each : container)
    {
        mylogS(" %d_%d ",each,pi);
    }
    mylogD("\n====================================================================");
}
