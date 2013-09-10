#pragma once
#include <string>
#include "common.h"
#include "mmysql.h"

class SpecialPush
{
    std::string _pushBaseDir;
    std::string _pidFile;
    std::string _baseDir;
    PSET _pidSet; 
    MYSQL* _sqlIns;
    int _sw;
    int _sh;
    uint32_t _srcPid;
    uint32_t _curPid;
    uint32_t _sqlSplit;
    uint32_t _dirSplit;
    
    bool _makeStaicSql(MYSQL* sqlIns ,uint32_t pid,const std::string& path,int sqlSplit);
    bool _makeDynamicSql(MYSQL* sqlIns ,uint32_t pid);
    bool _getPid();
    bool _setPid();



public:
    SpecialPush():_sqlIns(NULL){}
    ~SpecialPush();
    bool init(const ConfData& conf);
    void unInit();
    int getSpecialPush();
    int getAllCount();
    PSET SpecialPush::getSomeSpecial(int num);
};
