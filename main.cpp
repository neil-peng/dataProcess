#include "common.h"
#include "specialPush.h"
#include "./lib/lightConf/ConfigFile.h"
#include "adjust.h"
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
	Adjust* ins = new Adjust();
	//SpecialPush* spIns = new SpecialPush();
	


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
    return 0;
}
