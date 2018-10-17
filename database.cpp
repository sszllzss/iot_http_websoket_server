/*************************************************************************
# > File Name: database.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-01 21:32:36
# > Revise Time: 2018-10-17 11:42:31
 ************************************************************************/
#include<iostream>
#include<time.h>
#include<string.h>
#include "database.h"
#include "stdio.h"
#define MYSQL_HOST "172.27.16.9"
#define MYSQL_PORT 3306
#define MYSQL_USER "lotsszl"
#define MYSQL_PWD  "Qq5203344."
#define MYSQL_DBNAME "lotsszl"
#define MYSQL_MIN_CONNET_NUM 10
#define MYSQL_MAX_CONNET_NUM 1000
#define MYSQL_CONNET_TIMEOUT 3000

DataBase::DataBase()
{
    mysqlDB = new MysqlDB(MYSQL_DBNAME, MYSQL_USER, MYSQL_PWD, MYSQL_HOST, MYSQL_PORT, MYSQL_MIN_CONNET_NUM, MYSQL_MAX_CONNET_NUM , MYSQL_CONNET_TIMEOUT);
}
DataBase::~DataBase()
{
    if(mysqlDB != NULL)
        delete  mysqlDB;
}
int DataBase::AddUser(std::string user, std::string pwd, std::string nikename, std::string Email, int role )
{
    int rec= -1;
    if(user.empty() || pwd.empty() || nikename.empty())
        return rec;
    if(CheckUpUser(user) == 0)
        return rec;
    std::string sql;
    sql = "INSERT INTO user_table(`user`, pwd, nikename, Email, role) VALUES( '" 
        + user + "','" + pwd + "','" 
        + nikename + "','" + Email +"',";
    sql += std::to_string(role);
    sql += ")";
    if(mysqlDB->QueryNoRec(sql) > 0)
        rec = 0;
    return rec;
}
int DataBase::CheckUpUser(std::string user)
{
    int rec = -1;
    if(user.empty())
        return rec;
    std::string sql;
    Mysql_ROWS *rows = NULL;
    do
    {
        sql = "SELECT *  FROM user_table WHERE `user`='" + user + "'";
        if( (rows = mysqlDB->Query(sql)) == NULL)
        {
            break;
        }
        if(rows->size() > 0)
        {
            rec = 0;
        }
        delete rows;
    }
    while(0);
    return rec;
}
int DataBase::UserLogin(std::string user, std::string pwd)
{
    int rec= -1;
    if(user.empty() || pwd.empty())
        return rec;
    std::string sql;
    Mysql_ROW::iterator row;
    Mysql_ROWS *rows = NULL;
    do
    {
        sql = "SELECT `user` FROM user_table WHERE `user`='" + user + "' and pwd='" + pwd  + "'";
        if( (rows = mysqlDB->Query(sql)) == NULL)
        {
            break;
        }
        if(rows->size() >0)
        {
            row = rows->back()->find("user");
            if( row != rows->back()->end())
            {
                if(row.getValue().compare(user) == 0)
                {
                    rec = 0;
                }
            }
        }
    }
    while(0);
    return rec;

}
int DataBase::ReviseUserPwd(std::string user, std::string pwd ,std::string newPwd)
{
    int rec= -1;
    if(user.empty() || pwd.empty() || newPwd.empty())
        return rec;
    std::string sql;
    std::string err_str;
    if(UserLogin(user,pwd) == 0)
    {
        sql = "UPDATE  user_table SET pwd='"+ newPwd  +"' WHERE `user`='" + user + "'";
        if( (mysqlDB->QueryNoRec(sql)) > 0)
        {
            rec = 0;
        }
    }    
    return rec;
}
int DataBase::ReviseUserRole(std::string user, int role)
{
    int rec= -1;
    if(user.empty())
        return rec;
    std::string sql;
    std::string err_str;
    sql = "UPDATE  user_table SET role="; 
    sql += std::to_string(role);
    sql += " WHERE `user`='" + user + "'";
    if( (mysqlDB->QueryNoRec(sql)) > 0)
    {
        rec = 0;
    }
    return rec;

}
int DataBase::ReviseUserNikename(std::string user, std::string nikename)
{
    int rec=-1;
    if(user.empty()  || nikename.empty())
        return rec;
    std::string sql;
    std::string err_str;
    sql = "UPDATE  user_table SET nikename='"+ nikename  +"' WHERE `user`='" + user + "'";
    if( (mysqlDB->QueryNoRec(sql)) > 0)
    {
        rec = 0;
    }
    return rec;
}
int DataBase::ReviseUserEmail(std::string user, std::string Email)
{
    int rec= -1;
    if(user.empty() || Email.empty())
        return rec;
    std::string sql;
    std::string err_str;
    sql = "UPDATE  user_table SET Email='"+ Email  +"' WHERE `user`='" + user + "'";
    if( (mysqlDB->QueryNoRec(sql)) > 0)
    {
        rec = 0;
    }
    return rec;
}
int DataBase::AddSensor(std::string user, std::string sensorID , std::string key)
{
    int rec = -1;
    if(user.empty() || sensorID.empty() || key.empty())
        return rec;
    if(!CheckUpSensor(sensorID).empty())
        return  rec;
    std::string sql;
    sql = "INSERT INTO user_sensor(`user`, sensorID, `key`) VALUES( '" 
        + user + "','" + sensorID + "','" + key +"')"; 
    if(mysqlDB->QueryNoRec(sql) > 0)
        rec = 0;
    return rec;
}
std::string DataBase::CheckUpSensor(std::string sensorID)
{
    std::string rec;
    if(sensorID.empty())
        return rec;
    std::string sql;
    Mysql_ROW::iterator row;
    Mysql_ROWS *rows = NULL;
    do
    {
        sql = "SELECT `key`  FROM user_sensor WHERE sensorID='" + sensorID + "'";
        if( (rows = mysqlDB->Query(sql)) == NULL)
        {
            break;
        }
        if(rows->size() >0)
        {
            Mysql_ROW * row = rows->back();
            Mysql_ROW::iterator i =  row->find("key");
            rec = i.getValue();
        }
        delete rows;
    }
    while(0);
    return rec;


}
Mysql_strList * DataBase::GetUserSensorList(std::string user)
{
    Mysql_strList * rec = NULL;
    std::string sql;
    do
    {
        if(user.empty())
            break;
        sql = "SELECT sensorID  FROM user_sensor WHERE `user`='" + user + "'";
        Mysql_Res * res = mysqlDB->QueryRecRes(sql); 
        if(res == NULL)
            break;
        Mysql_Res::iterator i = res->find("sensorID");
        if(i != res->end())
            rec = new Mysql_strList(*i.getValue());
        delete res;
    }while(0);
    return rec;
}
int DataBase::DelSensor(std::string user, std::string sensorID)
{
    int rec = -1;
    if(user.empty() || sensorID.empty())
        return rec;
    std::string sql;
    if(CheckUpSensorIsUser(user, sensorID) == 0)
    {
        return DelSensor(sensorID);
    }
    return rec;
}
int DataBase::DelSensor(std::string sensorID)
{
    int rec = -1;
    if(sensorID.empty())
        return rec;
    std::string sql;
    sql = "DELETE FROM data_table WHERE sensorID='" + sensorID + "'";
    mysqlDB->QueryNoRec(sql);
    sql = "DELETE FROM user_sensor WHERE sensorID='" + sensorID + "'"; 
    if(mysqlDB->QueryNoRec(sql) > 0)
        rec = 0;
    return rec;
}

int DataBase::CheckUpSensorIsUser(std::string user, std::string sensorID)
{
    int rec = -1;
    if(user.empty() || sensorID.empty())
        return rec;
    std::string sql;
    Mysql_ROWS *rows = NULL;
    do
    {
        sql = "SELECT *  FROM user_sensor WHERE `user`='" + user + "' and sensorID='" +sensorID  +"'";
        if( (rows = mysqlDB->Query(sql)) == NULL)
        {
            break;
        }
        if(rows->size() > 0)
        {
            rec = 0;
        }
        delete rows;
    }
    while(0);
    return rec;
}
int DataBase::ReviseSensor(std::string user, std::string sensorID, std::string newKey)
{
    int rec = -1;
    if(user.empty() || sensorID.empty() || newKey.empty())
        return rec;
    std::string sql;
    sql = "UPDATE user_sensor SET `key`='" + newKey + "' WHERE `user`='" + user +"' and sensorID='"+ sensorID +"'"; 
    if(mysqlDB->QueryNoRec(sql) > 0)
        rec = 0;
    return rec;
}
int DataBase::ReviseSensor(std::string sensorID, std::string newKey)
{
    int rec = -1;
    if(sensorID.empty() || newKey.empty())
        return rec;
    std::string sql;
    sql = "UPDATE user_sensor SET `key`='" + newKey + "' WHERE sensorID='"+ sensorID +"'"; 
    if(mysqlDB->QueryNoRec(sql) > 0)
        rec = 0;
    return rec;
}

int DataBase::AddSensorData(std::string sensorID, std::string date, double illumination, double humidity, double temperature)
{
    int rec = -1;
    std::string sql;
    if(sensorID.empty() || date.empty())
        return rec;
    struct tm tmTemp;
    if(strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tmTemp) == NULL)
        return  rec;
    sql = "INSERT INTO data_table(sensorID, date_timer, illumination, humidity, temperature) VALUES('"+
            sensorID +"','"+ date +"',"+ std::to_string(illumination) +","+ std::to_string(humidity) +"," + std::to_string(temperature)  +")";
    if(mysqlDB->QueryNoRec(sql) > 0)
    {
        rec = 0;
    }
    return rec;
}
int DataBase::DelSensorAllData(std::string sensorID)
{
    int rec = -1;
    std::string sql;
    if(sensorID.empty())
        return rec;
    sql = "DELETE FROM data_table WHERE sensorID='"+ sensorID  +"'";
    rec = mysqlDB->QueryNoRec(sql);
    return rec;
}
int DataBase::DelSensorAllData(std::string user,std::string sensorID)
{
    int rec = -1;
    std::string sql;
    if(CheckUpSensorIsUser(user, sensorID) != 0)
        return rec;
    return DelSensorAllData(sensorID);
}
Mysql_Res * DataBase::GetUserAllSensorRes(std::string user,std::string startDate, std::string endDate)
{
    Mysql_Res *rec = NULL;
    std::string sql;
    struct tm tmTemp;
    if(user.empty() ||  startDate.empty() || endDate.empty())
        return rec;
    sql = "SELECT A.sensorID IdCode,A.illumination,A.humidity,A.temperature,A.date_timer `date` FROM data_table A INNER JOIN (SELECT sensorID FROM user_sensor WHERE `user`='" + user +"') B ON (A.sensorID = B.sensorID)";
    if(! (strcmp(startDate.c_str(),"0000-00-00 00:00:00") == 0 && (strcmp(endDate.c_str() ,"0000-00-00 00:00:00") == 0)))
    {
        if(strptime(startDate.c_str(), "%Y-%m-%d %H:%M:%S", &tmTemp) == NULL || strptime(endDate.c_str(), "%Y-%m-%d %H:%M:%S", &tmTemp) == NULL)
            return  rec;
        sql += " WHERE A.date_timer >='"+ startDate  +"' and A.date_timer <= '"+ endDate +"'";
    }

    return mysqlDB->QueryRecRes(sql);
        
}
Mysql_Res * DataBase::GetSensorRes(std::string sensorID, std::string startDate , std::string endDate)
{
    Mysql_Res *rec = NULL;
    std::string sql;
    struct tm tmTemp;
    if(sensorID.empty() ||  startDate.empty() || endDate.empty())
        return rec;
    sql = "SELECT A.sensorID IdCode,A.illumination,A.humidity,A.temperature,A.date_timer `date` FROM data_table A WHERE A.sensorID='"+ sensorID +"'";
    if(! (strcmp(startDate.c_str(),"0000-00-00 00:00:00") == 0 && (strcmp(endDate.c_str() ,"0000-00-00 00:00:00") == 0)))
    {
        if(strptime(startDate.c_str(), "%Y-%m-%d %H:%M:%S", &tmTemp) == NULL || strptime(endDate.c_str(), "%Y-%m-%d %H:%M:%S", &tmTemp) == NULL)
            return  rec;
        sql += " and A.date_timer >='"+ startDate  +"' and A.date_timer <= '"+ endDate +"'";
    }
    return mysqlDB->QueryRecRes(sql);
}
DataBase &DataBase::getInstance()
{
    static DataBase instance ;
    return instance;
}
/*//懒汉 线程安全 自动回收
  DataBase::GC DataBase::gc;
  DataBase * DataBase::instance = NULL;
  CMutex DataBase::instance_lock;
  DataBase * DataBase::getInstance()
  {
  if(instance == NULL)
  {
  instance_lock.lock();
  if(instance == NULL)
  {
  instance = new DataBase();
  }
  instance_lock.unlock();
  }
  return instance; 
  }
  */
