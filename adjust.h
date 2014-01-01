#pragma once
#include "mmysql.h"
#include "mmc.h"
#include <vector>
#include "specialPush.h"
//single thread



/*
*   all page indexs classify in 3 level  => a b & base
*   levelA --> levelB --> levelC
*   ^           ^         |
*   |           |         |
*   |-------------<--------
*   |
*   spe
*   memcached key: imgPI_1 <=> "12|234|345|34|"
*/
class Adjust
{
    MYSQL* _sqlIns;
    memcached_st* _mcIns;
    uint32_t _sqlSplit;
    SpecialPush _speIns;
    InfoStatus _status;
    DSET _baseVacancy;
    DSET _baseContainer;
    PSET _specSet;
    DSET _specDSet;
    int _debug;
    uint32_t _nowCount;
    uint32_t _lastCount;
    bool _initMysql(char* ip, int port,const char* user, const char* pwd,const char* db);
    bool _initMc(char* ip, int port );

    DSET _getPageIndexSet(int pageIndex);

    bool _updateEachpage(int pageIndex,const PSET &  newComer);
    bool _setbackBase(const DSET& setback);
    PSET _getGapSet(int pageIndex , int gapNum);
  
    PSET _getAllLuckFromBase(int luckNum);
    PSET _getALuckFromBase(int luckNum);
    PSET _adjustLevelA();
    PSET _adjustLevelB(const PSET& lastLeft);
    bool _adjustBase(const PSET & fillSet);
    std::string _makeMcIndex(const DSET& container );
    static const int LevelACount;
    static const int LevelBCount;
    static const int LevelAGAP;
    static const int LevelBGAP;
    static const int eachPage;
public: 
    Adjust():_sqlIns(NULL),_mcIns(NULL){
    }
    bool init(const ConfData&);// init sqlIns mcIns sepIns
    bool unInit();
    bool doAdjust();
    bool fillbackMc();
};
