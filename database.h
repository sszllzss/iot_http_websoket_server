/*************************************************************************
# > File Name: database.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-01 21:32:50
# > Revise Time: 2018-10-16 19:43:19
 ************************************************************************/

#ifndef _DATABASE_H
#define _DATABASE_H
// 编译请加 -std=c++11 选项
#include <mysql/mysql.h>
#include "CMutex.hpp"
#include "mysql_db.hpp"
class DataBase : private CMutex
{
private:
    MysqlDB *mysqlDB;

    DataBase(); 
    ~DataBase();
public:
    static DataBase &getInstance();
    
    int AddUser(std::string user, std::string pwd, std::string nikename, std::string Email = "", int role = 1);
    int CheckUpUser(std::string user);
    int ReviseUserPwd(std::string user, std::string pwd,std::string newPwd);
    int ReviseUserRole(std::string user, int role);
    int ReviseUserNikename(std::string, std::string nikename);
    int ReviseUserEmail(std::string user, std::string Email);
    int UserLogin(std::string user,std::string pwd);
    int AddSensor(std::string user, std::string sensorID,std::string key);
    std::string CheckUpSensor(std::string sensorID);
    Mysql_strList * GetUserSensorList(std::string user);
    int DelSensor(std::string user, std::string sensorID);
    int DelSensor(std::string sensorID);
    int CheckUpSensorIsUser(std::string user, std::string sensorID);
    int ReviseSensor(std::string user, std::string sensorID, std::string newKey);
    int ReviseSensor(std::string sensorID, std::string newKey);
    int AddSensorData(std::string sensorID, std::string date, double illumination, double humidity, double temperature);
    Mysql_Res * GetSensorRes(std::string sensorID,std::string startDate, std::string endDate);
    Mysql_Res * GetUserAllSensorRes(std::string user,std::string startDate, std::string endDate);
    int DelSensorAllData(std::string sensorID);
    int DelSensorAllData(std::string user, std::string sensorID);
    //懒汉 线程安全 自动回收
//private:
    /* static DataBase * instance; */
    /* static CMutex instance_lock; */
    /*
    class GC // 垃圾回收类  
    {   
    public:     
        GC(){
        }
        ~GC() 
        {     
            if (instance != NULL)
            {               
                delete instance;        
                instance = NULL;   
            }       
        }  
    }; 
    static GC gc;  //垃圾回收类的静态成员
    */

//public:
    /* static DataBase *getInstance(); */
};
#endif
