#include "specialPush.h"
#include "util.h"
#include <sys/types.h>
#include <dirent.h> 
#include <string.h>
#include "MyImage.h"

static int  EACHPAGE = 12;
template<typename T>
std::vector<T> subSET(std::vector<T>& leader, int count)
{
    std::vector<T> ret;
    int size = leader.size();
    if(count>size)
        return ret;
    for(int i=0; i< count;++i)
    {
        ret.push_back(leader.back());
        leader.pop_back();
    }
    return ret;
}

bool SpecialPush::init(const ConfData& conf)  
{
	if(NULL == (_sqlIns =  openMysql(conf.dbip.c_str(),conf.dbport,conf.dbuser.c_str(),conf.dbpwd.c_str(),conf.dbname.c_str())))
       return false;
    if(NULL ==  (_mcIns = openMc(conf.mcip.c_str(),conf.mcport)))
        return false;
    _pushBaseDir = conf.pushBaseDir;
    _pidFile = conf.pidPath;

    _baseDir = conf.sourceDir;
    _sqlSplit = conf.sqlSplit;
    _dirSplit = conf.dirSplit;
    _sw=conf.width;
    _sh= conf.height;
    _typeDir = conf.typeDir;
    return true;
}

bool SpecialPush::_makeStaicSql(uint32_t pid,const std::string& path,int sqlSplit)
{
    char sql[2048]={0};
    uint32_t sqlIndex = pid/sqlSplit;
    snprintf(sql,2048,"insert into info_static_%d values(%d,\"%s\",0,0)", sqlIndex,pid,path.c_str() );
    mylogD("spec make static sql: %s, sql : %d",sql,sqlSplit);
    bool ret =  doUpdateSql(_sqlIns,sql);
    return ret;
}

bool SpecialPush::_makeDynamicSql(uint32_t pid,uint32_t pageIndex)
{
    char sql[2048]={0};
    int weight = pid%5+1;
    snprintf(sql,2048,"insert into info_dynamic  values(%d,%d,%d)",pid,pageIndex,weight);
    /*
    mc 里面存入 uptW_pid -- weight
    */
    char mcKey[1024]={0};
    char mcVal[1024]={0};
    snprintf(mcKey,1024,"uptW_%d",pid);
    snprintf(mcVal,1024,"%d",weight);
    if(setMc(_mcIns,mcKey,mcVal)!=true)
    {
        mylogF("spec set uptW into memcache fail !!!");
        return false;
    }

    bool ret =  doUpdateSql(_sqlIns,sql);
    return ret;
}


bool SpecialPush::_makeStatusSql()
{
    char sql[2048]={0};
    snprintf(sql,2048,"update info_status set count = %d where uniq = 0",_curPid);
    bool ret =  doUpdateSql(_sqlIns,sql);
    return ret;
}

int SpecialPush::_queryTypeCount(uint32_t type)
{  
    char sql[2048]={0};
    int count;
    snprintf(sql,2048,"select count from info_status where uniq = %d",type);
    bool ret =  doQuerysql(_sqlIns,sql,count);
    if (!ret)
    {
       return -1;
    }
    return count;
}


bool SpecialPush::_insertTypeSql(uint32_t type, uint32_t pid, uint32_t pageIndex)
{
    char sql[2048]={0};
    snprintf(sql,2048,"insert into  info_type_%d values (%d , %d , 0)",type,pid,pageIndex);
    return doUpdateSql(_sqlIns,sql);
    return true;
}



bool SpecialPush::_updateTypeStatus(uint32_t type , uint32_t count)
{
    char sql[2048]={0};
    snprintf(sql,2048,"update info_status set count = %d where uniq = %d",count,type);
    return doUpdateSql(_sqlIns,sql);
}


bool SpecialPush::_getPid()
{
	FILE* file = NULL;
	if((file = fopen(_pidFile.c_str(),"r+"))==NULL)
	{
		mylogF("open pid file fail,in %s",_pidFile.c_str());
		return false;
	}
	char pidStr[1024]={0};
	fgets(pidStr,1024,file);
	_srcPid = atoi(pidStr);
    _curPid= _srcPid;
	fclose(file);
    mylogD("specialPush: get pid=%d",_srcPid);
    _lastPid = _srcPid;
	return true;
}

bool SpecialPush::_setPid()
{
	FILE* file = NULL;
	if((file = fopen(_pidFile.c_str(),"w+"))==NULL)
	{
		mylogF("open pid file fail,in  %s",_pidFile.c_str());
		return false;
	}
	char pidStr[1024]={0};
	snprintf(pidStr,1024,"%d",_curPid);
	fputs(pidStr,file);
	fclose(file);
    mylogD("specialPush: set pid=%d",_curPid);
	return true;
}

bool SpecialPush::addTypePush()
{
    uint32_t type = 0;
    for(auto each : _typeDir)
    {
        _baseDir = each;
        mylogD("%s  %s","input type dir: ",_baseDir.c_str());
        sleep(1);
        getSpecialPush(++type);   
    }
    return true;
}


int SpecialPush::getSpecialPush(uint32_t type)
{
	if(!_getPid())
		return false;
	struct dirent *dir_info = NULL;
    char filename[1024];
    uint32_t index=0;
    int ret=0;
    int typeCount = 0;
    
    if(type!=0)
        typeCount = _queryTypeCount(type);

    DIR* innerDir = opendir(_baseDir.c_str());
    if(innerDir==NULL)
    {    
        mylogF("spec innerDir is NULL, baseDir is %s, error %d",_baseDir.c_str(),errno); 
        return -1;
    }

    while(  NULL !=(dir_info=readdir(innerDir)) ) 
    {
        if((strcmp(dir_info->d_name,".")==0)||(strcmp(dir_info->d_name,"..")==0))
            continue;
        _curPid++;
        typeCount++;
        snprintf(filename,1024,"/%s",dir_info->d_name);
        std::string finPath =_baseDir+std::string(filename);
        std::string wh;
        char indexDir[1024]={0};
        char showDir[1024]={0};
        char cpid[1024]={0};
        index=_curPid/_dirSplit;
        snprintf(indexDir,1024,"/index%d/",index);
        snprintf(showDir,1024,"/show%d/",index);
        snprintf(cpid,1024,"%d_",_curPid);
        std::string smallDirFull = _pushBaseDir +  indexDir;
        std::string showDirFull = _pushBaseDir + showDir;
    
        MyImage proImg(finPath);  
        if(""==(wh = proImg.getWidthHeight()))    
        {
            mylogF("this pid getWidthHeight err : %d",_curPid);
            continue;
        }

        std::string sImgname = cpid + wh +".jpg";
        std::string showImgname = getMd5(sImgname); 

        std::string smallImg = smallDirFull + sImgname;
        std::string showImg = showDirFull + showImgname + ".jpg";
    
        mylogD("get smalldir: %s , smallImg: %s, showdir:%s, showImg:%s. pid: %d",smallDirFull.c_str(), smallImg.c_str(), showDirFull.c_str(),showImg.c_str() ,_curPid);
        _makeStaicSql(_curPid,sImgname,_sqlSplit);    
        uint32_t pageIndex = _curPid/EACHPAGE;
        _makeDynamicSql(_curPid,pageIndex);
        
        //说明走的是type添加的流程 
        if(type!=0)
        {
            pageIndex = typeCount/EACHPAGE;
            _insertTypeSql(type,_curPid,pageIndex);
        }
        
        if(! proImg.reSize(_sw,_sh) )
        {
            mylogF("this pid reSize err : %d",_curPid);
            continue;
        }
        
        if(! proImg.saveDstImg(smallImg,smallDirFull))
        {
            mylogF("this pid saveDstImg err : %d",_curPid);
            continue;
        }
       
        if(! proImg.saveSrcImg(showImg,showDirFull))
        {
            mylogF("this pid saveSrcImg err : %d",_curPid);
            continue;
        }

        _pidSet.push_back(_curPid);
        _pidDSet.push_back(DynamicInfo(_curPid,pageIndex,_curPid%5+1));
        ++ret;

    }
    
    if(!_setPid())
    {
        mylogF("update status[count] into pid file fail !!!");
        exit(1);
    }

    if(!_makeStatusSql())
    {
        mylogF("update status[count] into mysql fail !!!");
        exit(1);
    }

    if(type!=0)
    {
        _updateTypeStatus(type,typeCount);
    }
    if(closedir(innerDir)!=0)
    {
        mylogF("close file fail");
    }
    innerDir=NULL;
    mylog("spec: get getSomeSpecial done");
   
 	return ret;
}

int SpecialPush::getAllCount()
{
    return _curPid;
}

int SpecialPush::getLastCount()
{
    return _lastPid;
}

PSET SpecialPush::getSomeSpecial(int num)
{    
    return subSET(_pidSet,num);
}

PSET SpecialPush::getSpecSet()
{
    //mylogD("print spec pidSet");
    //printPset(_pidSet, -1);
    return _pidSet;
}

DSET SpecialPush::getSpecDSet()
{
    //mylogD("print spec pidDSet");
    //printDset(_pidDSet);
    return _pidDSet;
}

void SpecialPush::unInit()
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
}
SpecialPush::~SpecialPush()
{
    unInit();
}