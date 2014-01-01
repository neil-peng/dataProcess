#pragma once
#include <unistd.h>
#include <string>
#include <vector>

#define mylog(format,args...) do{\
    fprintf(stdout,format"\n",##args);\
}while(0)

#define mylogD(format,args...) do{\
    fprintf(stdout,"DEBUG: "format"\n",##args);\
}while(0)

#define mylogS(format,args...) do{\
        fprintf(stdout,format,##args);\
}while(0)


#define mylogF(format,args...) do{\
    fprintf(stdout,"FATAL: "format"\n",##args);\
}while(0)

#define mylogW(format,args...) do{\
    fprintf(stdout,"WARNNING: "format"\n",##args);\
}while(0)

#define PG 15
#define SQLLEN 2048
typedef std::vector<uint32_t> PSET;


struct ConfData
{
    //for store
    std::string dbip;
    int dbport;
    std::string dbuser;
    std::string dbpwd;
    std::string dbname;
    std::string mcip;
    int width;
    int height;
    int mcport;
    // for spec
    std::string pushBaseDir;
    std::string pidPath;
    std::string sourceDir;
    int sqlSplit;
    int dirSplit;
    bool isDebug;
    //for type
    bool isOnlyType;
    std::vector<std::string> typeDir;
};

struct DynamicInfo
{
    uint32_t pid;
    uint32_t pageIndex;
    int weight;
    DynamicInfo(uint32_t ppid, uint32_t ppageIndex,int wweight)
        :pid(ppid),pageIndex(ppageIndex),weight(wweight){}
    DynamicInfo(){}
    bool operator < (const DynamicInfo & in)
    {
        return weight < in.weight;
    }
};

struct InfoStatus
{
    uint32_t count;//pid 总数
    uint32_t indexCount;// 分了多少页
    void setInfo(uint32_t c, uint32_t ic)
    {
        count = c;
        indexCount = ic;
    }
    InfoStatus():count(0),indexCount(0){}
};


typedef std::vector<DynamicInfo> DSET;