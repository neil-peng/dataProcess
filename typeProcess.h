#pragma once
#include "specialPush.h"
#include "common.h"
class TypeProcess
{
	private:
		SpecialPush _speIns;
	public:
		TypeProcess(ConfData conf){
			if(!_speIns.init(conf))
			{
				mylogF("TypeProcess construct fail");
				exit(1);
			}
		}
		bool doAddType();
};