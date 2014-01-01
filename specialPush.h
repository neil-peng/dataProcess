#pragma once
#include <string>
#include "common.h"
#include "mmysql.h"
#include "mmc.h"


class SpecialPush
{
    std::string _baseDir;
    std::string _pushBaseDir;
    std::string _pidFile;
    std::vector<std::string> _typeDir;
    int _sqlSplit;
    int _dirSplit;
    PSET _pidSet; 
    DSET _pidDSet;
    MYSQL* _sqlIns;
    memcached_st* _mcIns;
    int _sw;
    int _sh;
    uint32_t _srcPid;
    uint32_t _curPid; 
    uint32_t _lastPid;    
    bool _makeStaicSql(uint32_t pid,const std::string& path,int sqlSplit);
    bool _makeDynamicSql(uint32_t pid,uint32_t pageIndex);
    bool _makeStatusSql();

    bool _insertTypeSql(uint32_t type, uint32_t pid,uint32_t pageIndex);
    bool _updateTypeStatus(uint32_t type,uint32_t count);
    int  _queryTypeCount(uint32_t type);

    bool _getPid();
    bool _setPid();



public:
    SpecialPush():_sqlIns(NULL){}
    ~SpecialPush();
    bool init(const ConfData& conf);
    void unInit();
    int getSpecialPush(uint32_t type = 0);
    bool addTypePush();
    int getAllCount();
    int getLastCount();
    PSET getSpecSet();
    PSET getSomeSpecial(int num);
    DSET getSpecDSet();
};
