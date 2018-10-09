/*************************************************************************
# > File Name: mysql_db.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-01 21:32:50
# > Revise Time: 2018-10-03 17:15:47
 ************************************************************************/

#ifndef _MYSQL_DB_H
#define _MYSQL_DB_H
#include <mysql/mysql.h>
#include <pthread.h>
#include <queue>
#include <map>
#include <list>
#include "CMutex.hpp"
class Mysql_strList:public std::list<std::string>
{};
class Mysql_Res : private CMutex
{
public:
    friend class iterator;
    friend class MysqlDB;
    class iterator
    {
        friend class Mysql_Res;
        friend bool operator==(iterator i1, iterator i2);
        friend bool operator!=(iterator i1, iterator i2);
    public:
        iterator operator++();//前缀++
        iterator operator--();
        iterator operator++(int);//后缀++
        iterator operator--(int);
        void operator=(const iterator & i);
        iterator();
        std::string getName();
        Mysql_strList * getValue();
    private:
        std::map<std::string, Mysql_strList *>::iterator iter;

        iterator(std::map<std::string, Mysql_strList *>::iterator i);
    };
    Mysql_Res();
    ~Mysql_Res();
    Mysql_Res::iterator begin();
    Mysql_Res::iterator end();
    Mysql_Res::iterator find(std::string);
    void insert(std::string name, Mysql_strList * value);
    void erase(Mysql_Res::iterator &i);
    void erase(std::string);
    void clear();
    size_t  size();
private:
    std::map<std::string, Mysql_strList *> *rows;

};
class Mysql_ROW :  private  CMutex
{
public:
    friend class iterator;
    friend class MysqlDB;
    class iterator
    {
        friend class Mysql_ROW;
        friend bool operator==(iterator i1, iterator i2);
        friend bool operator!=(iterator i1, iterator i2);
    public:
        iterator operator++();//前缀++
        iterator operator--();
        iterator operator++(int);//后缀++
        iterator operator--(int);
        void operator=(const iterator & i);
        iterator();
        std::string getName();
        std::string getValue();
    private:
        std::map<std::string, std::string>::iterator iter;

        iterator(std::map<std::string, std::string>::iterator i);
    };
    Mysql_ROW();
    ~Mysql_ROW();
    Mysql_ROW::iterator begin();
    Mysql_ROW::iterator end();
    Mysql_ROW::iterator find(std::string);
    void insert(std::string name, std::string value);
    void erase(Mysql_ROW::iterator &i);
    void erase(std::string);
    void clear();
    size_t  size();
private:
    std::map<std::string, std::string> *row;
};
class Mysql_ROWS : private CMutex
{
    friend class iterator;
    friend class MysqlDB;
public:
    class iterator
    {
        friend class Mysql_ROWS;
    public:
        iterator operator++();//前缀++
        iterator operator--();
        iterator operator++(int);//后缀++
        iterator operator--(int);
        void operator=(const iterator & i);
        Mysql_ROW * operator*();
        int operator==(iterator i);
        int operator!=(iterator i);
        iterator();
    private:
        std::list<Mysql_ROW *>::iterator iter;
        iterator(std::list<Mysql_ROW *>::iterator i);


    };
    Mysql_ROWS();
    ~Mysql_ROWS();
    void pop_back();
    void pop_front();
    void push_back(Mysql_ROW * row);
    void push_front(Mysql_ROW * row);
    Mysql_ROW * back();
    Mysql_ROW * front();
    Mysql_ROWS::iterator begin();
    Mysql_ROWS::iterator end();
    void erase(Mysql_ROWS::iterator &i);
    void clear();
    void insert(Mysql_ROWS::iterator &i, Mysql_ROW * row);
    size_t  size();
private:
    std::list<Mysql_ROW *> *rows;
};
class MysqlDB : private CMutex
{
private:
    std::queue<MYSQL *> conn_queue;
    static CMutex MysqlDB_connect_num_lock;
    static int MysqlDB_connect_num;
    int connect_timeout;//连接超时时间
    int min_connect_num;//最小连接数
    int max_connect_num;//最大连接数
    int connect_num;//当前连接数
    char *host;
    int port;
    char * user;
    char *passwd;
    char *dbname;
    MYSQL * connect();
    void close_conn(MYSQL * mysql);
public:
    MysqlDB(const char *db, const char * user, const char *passwd,const char *host = "localhost", const int port = 3306, int min_connect_num = 4, int max_connect_num = 1000, int connect_timeout = 3000); 
    void set_connect_timeout(int connect_timeout);
    void set_min_max_connect_num(int min_connect_num, int max_connect_num = 1000);
    ~MysqlDB();
    Mysql_ROWS * Query(std::string sql);
    int QueryNoRec(std::string sql);
    Mysql_Res * QueryRecRes(std::string sql);
};
#endif
