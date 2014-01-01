#pragma once
#include "./lib/mysql/include/mysql.h"
#include <string>
#include <vector>
#include "common.h"
typedef std::vector< std::pair<std::string,std::string > > SQLDATA ;
extern MYSQL* openMysql(const char* serv,int port,const char* user, const char* pwd,const char* db);
extern bool doDynamicSql(MYSQL *conn,char* sql, DSET& data);
extern bool doStatusSql(MYSQL *conn,char* sql,int& count);
extern bool doUpdateSql(MYSQL *conn,char* sql);
extern bool doStaticSql(MYSQL* conn,char* sql,std::string& retValue);
extern MYSQL* closeMysql(MYSQL* conn);
extern bool doQuerysql(MYSQL *conn,char* sql,int& count);