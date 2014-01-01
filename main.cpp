#include "common.h"
#include "specialPush.h"
#include "./lib/lightConf/ConfigFile.h"
#include "adjust.h"
#include "typeProcess.h"

ConfData loadConf()
{
	ConfData confData;
	ConfigFile conf("./conf/dp.conf");
    try
    {
    	confData.dbip = (std::string) conf.Value("DB","ip");
    	confData.dbport = (int)conf.Value("DB","port");
    	confData.dbpwd = (std::string) conf.Value("DB","pwd");
    	confData.dbuser = (std::string) conf.Value("DB","user");
    	confData.dbname = (std::string) conf.Value("DB","db");
    	confData.sqlSplit = (int) conf.Value("DB","sqlSplit");
    	confData.mcip = (std::string) conf.Value("MC","ip");
    	confData.mcport = (int) conf.Value("MC","port");
    	confData.sourceDir = (std::string) conf.Value("SPEC","sourceDir");
    	confData.pidPath = (std::string) conf.Value("SPEC","pidPath");
    	confData.pushBaseDir = (std::string) conf.Value("SPEC","pushBaseDir");
    	confData.width = (int) conf.Value("SPEC","sw");
    	confData.height = (int) conf.Value("SPEC","sh");
    	confData.dirSplit = (int) conf.Value("SPEC","dirSplit");
    	confData.isDebug = (int) conf.Value("SPEC","debug");
        confData.isOnlyType = (bool) conf.Value("CLASSIFY","only");
        int typeDirCount = (int) conf.Value("CLASSIFY","count");
        for(int i = 1 ; i<= typeDirCount;i++)
        {
            char tmp[128]="\0";
            snprintf(tmp,128,"%d",i);
            confData.typeDir.push_back((std::string)conf.Value("CLASSIFY",std::string(tmp)));
        }
    	
    }
    catch(char const* msg)
    {
    	mylogD("load conf catch exception : %s",msg);
        exit(0);
    }
    return confData;
}


int main(int argc, char** argv)
{
	ConfData confData = loadConf();
	//SpecialPush* spIns = new SpecialPush(confData);
    if(!confData.isOnlyType)
    {
        Adjust* ins = new Adjust();
    	if(!ins->init(confData))
    	{	
    		ins->unInit();
    		delete ins;
    		mylogF("adjust instance init fail");
    		return -1;
    	}    
        if(!ins->doAdjust())
    	{
    		mylogF("do adjust fail");
    		return -1;
    	}
    	if(!ins->fillbackMc())
    	{
    		mylogF("do fillback memecache fail");
    		return -1;
    	}
    	mylog("adjust finish");
        ins->unInit();
        delete ins;
    }
    else
    {
         TypeProcess* type = new TypeProcess(confData);
         type->doAddType();
         delete type;
    }



	
    return 0;
}
