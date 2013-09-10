#include "mmysql.h"
#include "stdio.h"
#include "stdlib.h"
MYSQL* openMysql(const char* serv,int port,const char* user, const char* pwd,const char* db)
{
   MYSQL *conn=NULL;;
  /* db configure*/
   conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, serv,
         user, pwd, db, port, NULL, 0)) {
      mylogF("MYSQL QUERY:%s", mysql_error(conn));
      exit(0);
   } 
   mysql_set_character_set(conn,"utf8"); 
   return conn;
}

//select * from 
bool doDynamicSql(MYSQL *conn,char* sql,DSET& container)
{
  
  MYSQL_ROW row;
  MYSQL_FIELD *field;
  MYSQL_RES *res=NULL;
  if (mysql_query(conn, sql)) {
      mylogF("MYSQL QUERY:%s", mysql_error(conn));
      exit(0);
  }
  res = mysql_store_result(conn);
  int num_fields=0;
  if( 3!= (num_fields =mysql_num_fields(res)) )
              return false;
	if(res!=NULL)
	{
      DynamicInfo tmp;
      while( row = mysql_fetch_row(res))
      { 
    	    tmp.pid = atoi(row[0]);
          tmp.pageIndex = atoi(row[1]);
          tmp.weight = atoi(row[2]);
          container.push_back(tmp);
      }
    	mysql_free_result(res);
		  return true;
	}
  
  return false;
}

bool doStaticSql(MYSQL* conn,char* sql,std::string& retValue )
{
  MYSQL_ROW row;
  MYSQL_FIELD *field;
  MYSQL_RES *res=NULL;
  if (mysql_query(conn, sql)) {
      mylogF("MYSQL QUERY:%s", mysql_error(conn));
      exit(0);
  }
  res = mysql_store_result(conn);
  int num_fields=0;
  if( 1!= (num_fields =mysql_num_fields(res)) )
              return false;
  if(res!=NULL)
  {
      DynamicInfo tmp;
      while( row = mysql_fetch_row(res))
          retValue = row[0];
      mysql_free_result(res);
      return true;
  }
  
  return false;

}

bool doStatusSql(MYSQL *conn,char* sql,int& count)
{
  
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    MYSQL_RES *res=NULL;
    if (mysql_query(conn, sql)) {
        mylogF("MYSQL QUERY :%s", mysql_error(conn));
        exit(0);
    }

    res = mysql_store_result(conn);
    int num_fields=0;
    if( 1!= (num_fields =mysql_num_fields(res)) )
              return false;
    if(res!=NULL)
    {
      row = mysql_fetch_row(res);
      count = atoi(row[0]);
    }
    mysql_free_result(res);
    return true;
   
}

//add more  future
bool doUpdateSql(MYSQL *conn,char* sql)
{
  MYSQL_ROW row;
  MYSQL_FIELD *field;
  MYSQL_RES *res=NULL;
  int index=0;
  if(mysql_query(conn, sql)) {
    mylogF("MYSQL QUERY:%s", mysql_error(conn));
    exit(0);
  }
  return true;
}




MYSQL* closeMysql(MYSQL* conn)
{
    mysql_close(conn);
    conn=NULL;
    return conn;
}


