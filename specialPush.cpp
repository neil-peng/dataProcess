#include "specialPush.h"
#include "util.h"
#include <sys/types.h>
#include <dirent.h> 
#include <string.h>
#include "MyImage.h"


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
    _pushBaseDir = conf.pushBaseDir;
    _pidFile = conf.pidPath;

    _baseDir = conf.sourceDir;
    _sqlSplit = conf.sqlSplit;
    _dirSplit = conf.dirSplit;
    _sw=conf.width;
    _sh= conf.height;

    return true;
}


bool SpecialPush::_makeStaicSql(MYSQL* sqlIns ,uint32_t pid,const std::string& path,int sqlSplit)
{
    char sql[2048]={0};
    uint32_t sqlIndex = pid/sqlSplit;
    snprintf(sql,2048,"insert into info_static_%d values(%d,\"%s\",0,0)", sqlIndex,pid,path.c_str() );
    DSET res; 
    bool ret =  doDynamicSql(sqlIns,sql,res);
    return ret;
}

bool SpecialPush::_makeDynamicSql(MYSQL* sqlIns ,uint32_t pid)
{
    char sql[2048]={0};
    snprintf(sql,2048,"insert into info_dynamic  values(%d,0,0)",pid);
    DSET res; 
    bool ret =  doDynamicSql(sqlIns,sql,res);
    return ret;
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
	return true;
}

int SpecialPush::getSpecialPush()
{
	if(!_getPid())
		return false;
	struct dirent *dir_info = NULL;
    char filename[1024];
    uint32_t index=0;

    int ret=0;
    static DIR* innerDir = opendir(_baseDir.c_str());
    if(innerDir==NULL)
        return -1;

    while(  NULL !=(dir_info=readdir(innerDir)) ) 
    {

        if((strcmp(dir_info->d_name,".")==0)||(strcmp(dir_info->d_name,"..")==0))
            continue;
        _curPid++;
        snprintf(filename,1024,"/%s",dir_info->d_name);
        std::string finPath = _baseDir+std::string(filename);
        std::string wh;
        char indexDir[1024]={0};
        char showDir[1024]={0};
        char cpid[1024]={0};
        int index=_curPid/_dirSplit;
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
        _makeStaicSql(_sqlIns,_curPid,smallImg,_sqlSplit);    
        _makeDynamicSql(_sqlIns,_curPid);

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
        ++ret;
    }

     _setPid();
   
 	return ret;
}

int SpecialPush::getAllCount()
{
    return _curPid;
}
PSET SpecialPush::getSomeSpecial(int num)
{    
    return subSET(_pidSet,num);
}

void SpecialPush::unInit()
{
	closeMysql(_sqlIns);
}
SpecialPush::~SpecialPush()
{
    unInit();
}