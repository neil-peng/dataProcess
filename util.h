#pragma once
#include <string>
#include <vector>
#include "common.h"

extern int getRandInt(int ,int);
extern std::string getRandString(int , int); 
extern  std::string getMd5(const std::string& in);
extern void printDset(const DSET& container);
extern void printPset(const PSET& container, int pi);