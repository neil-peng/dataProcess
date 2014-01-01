#include "adjust.h"
#include "util.h"
#include <algorithm> 
#include <set>

static const int Adjust::LevelACount = 30;
static const int Adjust::LevelBCount = 30;
static const int Adjust::LevelAGAP = 6;
static const int Adjust::LevelBGAP = 9;
static const int Adjust::eachPage = 12;

template<typename T>
std::vector<T> subSET(std::vector<T>& leader, int count)
{
    std::vector<T> ret;
    int size = leader.size();
    if(count>size)
    {
        mylogF("subSet err");
        return ret;
    }
    for(int i=0; i< count;++i)
    {
        ret.push_back(leader.back());
        leader.pop_back();
    }
    return ret;
}

template<typename T>
void addSET(std::vector<T>& leader, const std::vector<T>& follower)
{
    for(T each : follower)
        leader.push_back(each);
}

static DSET psetTodset(const PSET& input, int pI)
{
    DSET ret;
    for(uint32_t each : input)
    {
        DynamicInfo tmp;
        tmp.pid=each;
        tmp.pageIndex = pI;
        ret.push_back(tmp);
    }
    return ret;
}

bool Adjust::init(const ConfData& conf)
{
    if(!_initMysql(conf.dbip.c_str(),conf.dbport,conf.dbuser.c_str(),conf.dbpwd.c_str(),conf.dbname.c_str()))
        return false;
    if(!_initMc(conf.mcip.c_str(),conf.mcport))
        return false;
   // _status = _getInfoStatus();
   // if(_status.size()==0)
   //     return false;
    _sqlSplit = conf.sqlSplit;
    _debug = conf.isDebug;
    if(_speIns.init(conf))
        return true;
    
    return false;
}

bool Adjust::unInit()
{
    if(_sqlIns!=NULL)
    {
        closeMysql(_sqlIns);
        _sqlIns=NULL;
    }
    if(_mcIns!=NULL)
    {
        closeMc(_mcIns);
        _mcIns=NULL;
    }
    return true;
}

bool Adjust::_initMysql(char* ip, int port,const char* user, const char* pwd,const char* db)
{
    if(NULL == (_sqlIns =  openMysql(ip,port,user,pwd,db)))
       return false;
    return true;
}

bool Adjust::_initMc(char* ip, int port )
{
    if(NULL ==  (_mcIns = openMc(ip,port)))
        return false;
    return true;
}


DSET Adjust::_getPageIndexSet(int pageIndex)
{
    char sql[SQLLEN]={0};
    DSET container;
    snprintf(sql,SQLLEN,"select pid ,page_index ,weigth from info_dynamic where page_index = %d ;",pageIndex);
    if( false == doDynamicSql(_sqlIns,sql,container))
    {
       mylogD("print get pageIndex set print dset: %d",pageIndex);
       if(_debug)
            printDset(container);
       mylogD("do dynamic sql err");
       container.clear(); 
       return container;
    }
   // mylogD("container size:%d,sql : %s",container.size(),sql);
    return container;
}


bool Adjust::_updateEachpage(int pageIndex,const PSET &  newComer)
{
    bool ret = true;
    //get mc the origin pindex <=> array pid
    //update relative pid
    mylogD("print _updateEachpage");
    printPset(newComer,pageIndex);
    for(uint32_t each : newComer)
    {
        char sql[SQLLEN]={0};
        snprintf(sql,SQLLEN,"update info_dynamic set page_index = %d where pid =%d ;"
            ,pageIndex,each);
        if(doUpdateSql(_sqlIns,sql)!=true)
            ret = false;
        if(_debug)
            mylogD("update each page index:%d,sql:%s, ret:%d",pageIndex,sql,ret);
    }
    return ret;
}

bool Adjust::_setbackBase(const DSET& setback)
{
    bool ret = true;
    mylogD("print set back base dset");
    if(_debug)
        printDset(setback);
    for(DynamicInfo each: setback)
    {
        char sql[SQLLEN]={0};
        uint32_t pageIndex = each.pageIndex;
        uint32_t pid = each.pid;
        snprintf(sql,SQLLEN,"update info_dynamic set page_index = %d where pid =%d ;"\
            ,pageIndex,pid);
        doUpdateSql(_sqlIns,sql);
        mylogD("set back base:%s",sql);
    }
    return ret;
}

/*
* 改进如何更随机或者选则会更好??
*/
PSET Adjust::_getGapSet(int pageIndex , int gapNum)
{
    DSET pISet =  _getPageIndexSet(pageIndex);
    // sort pISet ,the lastest weight pids, if more than gapNum ,get rand
    std::sort(pISet.begin(), pISet.end(), std::less<DynamicInfo>());
    PSET ret;
    ret.clear();
    if(pISet.size()<gapNum)
    {
        mylogF("getGap too small , err in %s:%d, piset: %d,gapNum:%d, pageIndex:%d",__func__,__LINE__,pISet.size(),gapNum,pageIndex);
        return ret;
    }
    for(int i=0;i<gapNum;++i)
    {
        ret.push_back(pISet[i].pid);
    }
    return ret;
}

/*
*   get bases pid list, and  all index from 0 ---
*
*/
PSET Adjust::_getAllLuckFromBase(int luckNum)
{
    PSET ret;
    ret.clear();
    uint32_t beg;
    uint32_t end;
    if(luckNum <= 0)
    {
        mylogF("get all luck err");
        return ret;
    }

    int count = _lastCount;
    char sql[1024]="0";
    //获取上一次目前图片总数
    /*
    snprintf(sql,1024,"select count from info_status where uniq = 0;");
    doStatusSql(_sqlIns,sql,count);
    */

    mylogD("last all pid count is %d",count);

    if(count%eachPage==0)
        end = count/eachPage -1;
    else
      //  end = count/eachPage;
        end=count/eachPage;
    beg = LevelACount + LevelBCount;
    if(beg>=end)
    {
        mylogF("inner logic err in %s:%d, beg : %d, end :%d",__func__,__LINE__,beg,end);
        return ret;
    }
    //random to get some pages in bases,each one get a pid
    mylogD("get from base range: %d -- %d",beg,end);
    std::set<uint32_t> forUniq;
    for(int i=0;i<luckNum/5+1;i++)
    {
        uint32_t pIndex = getRandInt(beg,end-1);//the last may be not have 2 ,注意pid不能重复
        while(forUniq.insert(pIndex).second==false)
        {
            pIndex = getRandInt(beg,end-1);
        }

        PSET luckSet = _getGapSet((int)pIndex,5);
        if(luckSet.size()!=5)
        {
            mylogD("getAllLuckFromBas happen err, size err, get gap is %d",luckSet.size());
            ret.clear();
            exit(0);
            return ret;
        }    
               
        ret.push_back(luckSet[0]);
        ret.push_back(luckSet[1]);
        ret.push_back(luckSet[2]);
        ret.push_back(luckSet[3]);
        ret.push_back(luckSet[4]);
        addSET(_baseContainer,psetTodset(luckSet,pIndex));
    }
    
    addSET(_baseContainer,_specDSet);
    addSET(ret,_specSet);
    mylogD("print get from base dset");
    if(_debug)
        printDset(_baseContainer);
    return ret;
}

PSET Adjust::_getALuckFromBase(int luckNum)
{
    PSET lset;
    lset.clear();
    if(_baseContainer.size()<luckNum)
    {
        mylogF("get a luck from base fail!!!");
        exit(1);
        //return lset;
    }   
    
    for(int i=0;i<luckNum;++i)
    {       
        DynamicInfo tmp = _baseContainer.back();      
        _baseVacancy.push_back(tmp);
        lset.push_back(tmp.pid);
        _baseContainer.pop_back();
    }

    return lset;
}


PSET Adjust::_adjustLevelA()
{
    PSET retLeft;
    retLeft.clear();
    int spcialCount;
    if((spcialCount = _speIns.getSpecialPush())<0)
        spcialCount = 0;
    mylogD("spe dir return :%d",spcialCount); 
    _nowCount = _speIns.getAllCount();
    _lastCount = _speIns.getLastCount();
    _specSet = _speIns.getSpecSet();
    _specDSet = _speIns.getSpecDSet();
    if(_debug)
    {
        mylogD("print the spec pset, index = -1 for nothing");
        printPset(_specSet,-1);
    }
    mylogD("GET PID COUNT : NOW:%d, LAST:%d",_nowCount,_lastCount);

    //uint32_t needBase = LevelACount*LevelAGAP + LevelBCount*LevelBGAP - spcialCount;
    uint32_t needBase = LevelBCount*LevelBGAP + LevelACount*LevelAGAP;
    /*
    * 防止频繁的更新， spec 推送目录不大于100张 ,确保新增都替换到levelA中
    */
    if(spcialCount>100)
    {
        mylogF("check the special dir , too many imgs!");
        exit(0);
    }
    mylogD("all need from base :%d",needBase);
    _getAllLuckFromBase(needBase);

    int gapCount=LevelAGAP;//6
    for(int pageIndex =0 ; pageIndex <LevelACount;pageIndex++)
    {
        PSET newComer;
        newComer.clear();
        PSET gapSet = _getGapSet(pageIndex,gapCount);
        mylogD("print adjust A , get gap index:%d, pset",pageIndex); 
        if(_debug)
            printPset(gapSet,pageIndex);
        mylogD("adjust A index: %d, getGapSize: %d",pageIndex,gapSet.size());
        if(gapSet.size()<gapCount)
        {
            retLeft.clear();
            return retLeft;
        }       
       /*
        if( spcialCount > 0 )
        {
            int eachGapLeft = gapCount  >= spcialCount ?   (gapCount-spcialCount) : 0 ;
            if( eachGapLeft > 0 )
            {
               addSET( newComer ,_getALuckFromBase(eachGapLeft));
               addSET( newComer ,_speIns.getSomeSpecial(spcialCount));
               spcialCount = 0;
            }
            else 
            {
               addSET( newComer ,_speIns.getSomeSpecial(gapCount));  
               spcialCount -= gapCount;
            }
        }
        else
       */
        //{
        addSET( newComer ,_getALuckFromBase(gapCount));
        //}
        if(newComer.size()!=gapCount)
            mylogD("logic err in %s:%d, newComer:%d, gapCount:%d  ",__func__,__LINE__,newComer.size(),gapCount);   
        _updateEachpage(pageIndex,newComer);
        addSET(retLeft,gapSet);     
    }
   
    return retLeft;
}


PSET Adjust::_adjustLevelB(const PSET& lastLeft)
{
    //have nothing with the special ,cause the special will not have more than 100
    PSET retLeft;
    retLeft.clear();
    if(lastLeft.size()==0)
    {
        mylogF("err in adjustB, input in null");
        return retLeft;
    }
    PSET levelALeft = lastLeft;
    uint32_t gapCount = LevelBGAP;//9 
    uint32_t lastCount = levelALeft.size();
    mylogD("A left size: %d",lastCount);

    for(int pageIndex =LevelACount ;pageIndex< (LevelACount+LevelBCount) ;pageIndex++)
    { 
       PSET newComer;
       PSET gapSet = _getGapSet(pageIndex,gapCount); 

       mylogD("adjust b , get gap index:%d",pageIndex);
       if(_debug) 
            printPset(gapSet,pageIndex);
       if(lastCount>0)
       {
            uint32_t eachGapLeft = gapCount  > lastCount ?  (gapCount - lastCount) : 0 ;
            if(eachGapLeft > 0 )
            {
                addSET( newComer , _getALuckFromBase(eachGapLeft));
                addSET( newComer , subSET(levelALeft, lastCount));
                lastCount = 0;
            }
            else
            {
                addSET( newComer , subSET(levelALeft, gapCount));
                lastCount -=gapCount;
            }
       }
       else
       {
             addSET( newComer , _getALuckFromBase(gapCount));
       }
       _updateEachpage(pageIndex,newComer);
       addSET(retLeft,gapSet);   
      

    }

    if(levelALeft.size()>0)
    {
        addSET(retLeft,levelALeft); 
        mylogF("logic err in %s:%d",__func__,__LINE__);      
    }

    return retLeft;
}

bool Adjust::_adjustBase(const PSET & fillSet)
{
    if(fillSet.size()==0)
    {
        mylogF("err in adjustBase ,input is null");
        return false;
    }

    if(_baseVacancy.size() != fillSet.size())
        mylogF("err logic err in %s:%d, val:%d , fillset:%d",__func__,__LINE__,_baseVacancy.size(),fillSet.size());

    int size = _baseVacancy.size();
    for(int i=0; i<size;i++)
    {
        _baseVacancy[i].pid = fillSet[i];
    }
    return _setbackBase(_baseVacancy);//会多出一些
    
}

bool Adjust::doAdjust()
{    
    return _adjustBase(_adjustLevelB(_adjustLevelA()));//得到了要回填给level3(base)的集合
}

std::string Adjust::_makeMcIndex(const DSET& container )
{
    std::string ret;
    for(DynamicInfo each : container)
    {
        char sql[128]={0};
        uint32_t pid = each.pid;
        std::string picInfo;
        /*
        * 123_330_234
        */
        snprintf(sql,128,"select index_path from info_static_%d where pid = %d ",pid/_sqlSplit,pid);
        //snprintf(spid,128,"%d",each.pid);
        if(_debug)
            mylogD("sql: %s",sql);
        if(doStaticSql(_sqlIns,sql,picInfo)!=true)
            continue;
        //mylogD("pid: %d ,picInfo:%s ",pid,picInfo.c_str());
        ret+= picInfo+"|";
        
    }
    mylogD("mc picInfo: %s",ret.c_str());
    return ret;
}

/*
* select all pageindex from mysql ,set back to mc
*/
bool Adjust::fillbackMc()
{
   // uint32_t indexCount = _speIns.getAllCount();
    uint32_t indexCount = 0;

    int count = 0;
    char sql[1024]="0";
    snprintf(sql,1024,"select count from info_status;");
    doStatusSql(_sqlIns,sql,count);
    
    if(count%eachPage==0)
        indexCount = count/eachPage -1;
    else
        indexCount=count/eachPage;

    for(int index=0;index<indexCount;++index)
    {
        char pIndex[128]={0};
        snprintf(pIndex,128,"im_%d",index);
        DSET container = _getPageIndexSet(index);
        if(container.size()==0)
            return false;
        std::string pidSet  = _makeMcIndex(container); 
        if(!setMc(_mcIns,pIndex,pidSet.c_str()))
        {
            mylogD("set back mc fail");
            return false;
        }
    }
    return true;
}
