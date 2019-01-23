/*************************************************************************
# > File Name: mysql_db.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-01 21:32:36
# > Revise Time: 2018-12-05 10:04:53
 ************************************************************************/
#include<iostream>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"mysql_db.hpp"
int MysqlDB::MysqlDB_connect_num = 0;
CMutex MysqlDB::MysqlDB_connect_num_lock;
MysqlDB::MysqlDB(const char *db, const char * user, const char *passwd,const char *host, const int port , int min_connect_num, int max_connect_num, int connect_timeout)
{
    std::string err_str;
    do
    {

        MysqlDB_connect_num_lock.lock();
        if(MysqlDB_connect_num == 0)
        {
            mysql_library_init(0, NULL, NULL);

        }
        MysqlDB_connect_num_lock.unlock();
        this->dbname = (char *)malloc(strlen(db));
        this->user = (char *)malloc(strlen(user));
        this->passwd = (char *)malloc(strlen(passwd));
        this->host = (char * )malloc(strlen(host));
        if(this->dbname == NULL || this->user == NULL 
           || this->passwd == NULL || this->host == NULL)
        {
            err_str += "malloc fail!";
            break;
        }
        strcpy(this->dbname, db);
        strcpy(this->user, user);
        strcpy(this->passwd, passwd);
        strcpy(this->host, host);
        this->port = port;
        this->connect_timeout = connect_timeout;
        this->min_connect_num = min_connect_num;
        this->max_connect_num = max_connect_num;
        this->connect_num = 0;
        int timeout = 0;
        for(int i = 0;i < min_connect_num ;)
        {
            MYSQL * mysql;
            if((mysql =  connect()) != NULL)
            {
                conn_queue.push(mysql);
                connect_num++;
                timeout = 0;
                i++;
            }
            else
            {
                timeout++;
            }
            if(timeout > 10)
            {
                break;
            }
        }
        if(connect_num < min_connect_num)
        {
            err_str += "connectpool carter  connect timeout! ";
            break;
        }

    }while(0);
    if(!err_str.empty())
    {
        Free();
        throw err_str;
    }
}
Mysql_Res * MysqlDB::QueryRecRes(std::string sql)
{
    MYSQL * mysql = NULL;
    Mysql_Res *rows = NULL;
    MYSQL_ROW *Row;
    MYSQL_RES *result;
    MYSQL_FIELD *field;
    unsigned int num_rows;
    std::string err_str;
    do{
        lock();
        if(!conn_queue.empty())
        {
            mysql = conn_queue.front();
            conn_queue.pop();
        }
        unlock();
        if(mysql == NULL)
        {
            mysql = connect();
            if(mysql == NULL)
            {
                err_str += "connect mysql error!";
                throw err_str;
                return NULL;
            }
        }
        if(mysql_ping(mysql) != 0)
        {
            err_str += "The connection is disconnected and cannot be reconnected:";
            err_str += mysql_error(mysql);
            close_conn(mysql);
            throw  err_str;
            return NULL;
        }
        if(mysql_real_query(mysql, sql.c_str(), sql.size()) != 0 )
        {
            err_str += "Mysql query fali:";
            err_str += mysql_error(mysql);
            break;
        }
        result = mysql_store_result(mysql);
        if(result == NULL)
        {
            if(mysql_field_count(mysql) != 0)
            {
                err_str += "fetch result failed:";
                err_str += mysql_error(mysql);
            }   
            break;
        }       
        num_rows = mysql_num_rows(result);
        if(num_rows == 0)
        {
            mysql_free_result(result);
            break;
        }
        rows = new Mysql_Res();
        if(rows == NULL)
        {
            err_str += "new Mysql_Res() fail!";
            mysql_free_result(result);
            break;
        }
        Row = (MYSQL_ROW *)malloc(sizeof(MYSQL_ROW)*num_rows);
        if(Row == NULL)
        {
            err_str += "MYSQL_ROW malloc fail!";
            mysql_free_result(result);
            delete rows;
            rows = NULL;
            break;

        }
        Mysql_strList * column = NULL;
        unsigned int i, j;
        for(i = 0;i < num_rows;i++)
        {
            Row[i] = mysql_fetch_row(result);
            if(Row[i]==NULL)
            {
                err_str += "get noe row fail:";
                err_str += mysql_error(mysql);
                break;
            }
        }
        if(i< num_rows)
        {
            mysql_free_result(result);
            delete rows;
            rows = NULL;
            free(Row);
            break;
        }
        j = 0;
        while((field = mysql_fetch_field(result)) != NULL)
        {
            column = new Mysql_strList();
            if(column == NULL)
            {
                err_str += "new Mysql_strList() fail!";
                break;
            }
            for(i = 0;i < num_rows;i++)
            {
                column->push_back(Row[i][j]);
            }
            rows->insert(field->name, column);  
            j++;
        }
        if(field != NULL)
        {
            mysql_free_result(result);
            delete rows;
            rows = NULL;
            free(Row);
            break;
        }
        mysql_free_result(result);
        free(Row);
    }while(0);
    lock();
    if(connect_num > max_connect_num)
    {
        unlock();
        close_conn(mysql);
    }
    else
    {
        conn_queue.push(mysql);
        unlock();
    }
    if(!err_str.empty())
        throw err_str;
    return rows;
}

Mysql_ROWS * MysqlDB::Query(std::string sql)
{
    MYSQL * mysql = NULL;
    Mysql_ROWS *rows = NULL;
    MYSQL_ROW Row;
    MYSQL_RES *result;
    MYSQL_FIELD *field;
    unsigned int num_rows;
    std::string err_str;
    do{
        lock();
        if(!conn_queue.empty())
        {
            mysql = conn_queue.front();
            conn_queue.pop();
        }
        unlock();
        if(mysql == NULL)
        {
            mysql = connect();
            if(mysql == NULL)
            {
                err_str += "connect mysql error!";
                throw err_str;
                return NULL;
            }
        }
        if(mysql_ping(mysql) != 0)
        {
            err_str += "The connection is disconnected and cannot be reconnected:";
            err_str += mysql_error(mysql);
            close_conn(mysql);
            throw  err_str;
            return NULL;
        }
        if(mysql_real_query(mysql, sql.c_str(), sql.size()) != 0 )
        {
            err_str += "Mysql query fali:";
            err_str += mysql_error(mysql);
            break;
        }
        result = mysql_store_result(mysql);
        if(result == NULL)
        {
            if(mysql_field_count(mysql) != 0)
            {
                err_str += "fetch result failed:";
                err_str += mysql_error(mysql);
            }   
            break;
        }       
        num_rows = mysql_num_rows(result);
        if(num_rows == 0)
        {
            mysql_free_result(result);
            break;
        }
        rows = new Mysql_ROWS();
        if(rows == NULL)
        {
            err_str += "new Mysql_ROWS() fail!";
            mysql_free_result(result);
            break;
        }
        Mysql_ROW * row = NULL;
        unsigned int i;
        for(i = 0;i<num_rows;i++)
        {
            row = new Mysql_ROW();
            if(row == NULL)
            {
                err_str += "new Mysql_ROW() fail!";
                break;
            }
            Row = mysql_fetch_row(result);
            if(Row==NULL)
            {
                err_str += "get noe row fail:";
                err_str += mysql_error(mysql);
                break;
            }
            mysql_field_seek(result, 0);
            int j = 0;
            while((field = mysql_fetch_field(result)) != NULL)
            {
                row->insert(field->name, Row[j]);
                j++;
            }
            rows->push_back(row);
        }
        if(i != num_rows)
        {
            mysql_free_result(result);
            delete rows;
            rows = NULL;
            break;
        }
        mysql_free_result(result);

    }while(0);
    lock();
    if(connect_num > max_connect_num)
    {
        unlock();
        close_conn(mysql);
    }
    else
    {
        conn_queue.push(mysql);
        unlock();
    }
    if(!err_str.empty())
        throw err_str;
    return rows;
}
int MysqlDB::QueryNoRec(std::string sql)
{
    MYSQL * mysql = NULL;
    int rowcount = 0;
    std::string err_str;
    do{
        lock();
        if(!conn_queue.empty())
        {
            mysql = conn_queue.front();
            conn_queue.pop();
        }
        unlock();
        if(mysql == NULL)
        {
            mysql = connect();
            if(mysql == NULL)
            {
                err_str += "connect mysql error!";
                throw err_str;
                return -1;
            }
        }
        if(mysql_ping(mysql) != 0)
        {
            err_str += "The connection is disconnected and cannot be reconnected:";
            err_str += mysql_error(mysql);
            close_conn(mysql);
            throw  err_str;
            return -1;
        }
        if(mysql_real_query(mysql, sql.c_str(), sql.size()) != 0 )
        {
            err_str += "Mysql query fali:";
            err_str += mysql_error(mysql);
            break;
        }
        rowcount = mysql_affected_rows(mysql);
        if(rowcount == -1)
            err_str += "Affects row-count reading errors!";
    }while(0);
    lock();
    if(connect_num > max_connect_num)
    {
        unlock();
        close_conn(mysql);
    }
    else
    {
        conn_queue.push(mysql);
        unlock();
    }
    if(!err_str.empty())
        throw err_str;
    return rowcount;
}
void MysqlDB::Free()
{
    lock();
    if(this->dbname == NULL)
        free(this->dbname);
    if(this->user == NULL)
        free(this->user);
    if(this->passwd == NULL)
        free(this->passwd);
    if(this->host == NULL)
        free(this->host);
    while(1)
    {
        if(conn_queue.empty())
        {
            break;
        }
        MYSQL * mysql =  (MYSQL *)conn_queue.front();
        conn_queue.pop();
        if(mysql != NULL)
        {
            unlock();
            close_conn(mysql);
            lock();
        }
    }
    unlock();
    MysqlDB_connect_num_lock.lock();
    if(MysqlDB_connect_num == 0)
    {
        MysqlDB_connect_num_lock.unlock();
        mysql_library_end();
        return;
    }
    MysqlDB_connect_num_lock.unlock();

}
MysqlDB::~MysqlDB()
{
    Free();
}
void MysqlDB::set_connect_timeout(int connect_timeout)
{
    lock();
    this->connect_timeout = connect_timeout;
    unlock();
}
void MysqlDB::set_min_max_connect_num(int min_connect_num, int max_connect_num)
{
    lock();
    this->min_connect_num = min_connect_num;
    this->max_connect_num = max_connect_num;
    unlock();
}
MYSQL *  MysqlDB::connect()
{   
    std::string err_str;
    MYSQL * mysql = NULL;
    mysql = (MYSQL *)malloc(sizeof(MYSQL)); 
    if(mysql == NULL)
    {
        err_str += "Mysql malloc is error!";
        throw  err_str;
        return NULL;
    }
    lock();
    mysql_thread_init();
    if(mysql_init(mysql) == NULL)
    {
        err_str += "Mysql init is error!";
        throw  err_str;
        mysql_thread_end();
        unlock();
        free(mysql);
        return NULL;
    }
    my_bool reconnect = 0;
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &reconnect);//自动重连
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &this->connect_timeout);
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "UTF8");
    if(!mysql_real_connect(mysql, host, user, passwd, dbname, port, NULL, CLIENT_MULTI_STATEMENTS))
    {
        err_str += "Mysql connect is  error!";
        err_str += mysql_error(mysql);
        throw  err_str;
        mysql_thread_end();
        unlock();
        free(mysql);
        return NULL;
    }
    MysqlDB_connect_num_lock.lock();
    MysqlDB_connect_num++;
    MysqlDB_connect_num_lock.unlock();
    unlock();
    return mysql;
}
void MysqlDB::close_conn(MYSQL * mysql)
{
    lock();
    mysql_close(mysql);
    free(mysql);
    MysqlDB_connect_num_lock.lock();
    MysqlDB_connect_num--;
    MysqlDB_connect_num_lock.unlock();
    unlock();
}
Mysql_ROW::iterator Mysql_ROW::iterator::operator++()//前缀++
{
    Mysql_ROW::iterator i(++this->iter);
    return i;
}
Mysql_ROW::iterator Mysql_ROW::iterator::operator++( int)//后缀++
{
    Mysql_ROW::iterator i(this->iter++);
    return i;
}
Mysql_ROW::iterator Mysql_ROW::iterator::operator--()//前缀--
{
    Mysql_ROW::iterator i(--this->iter);
    return i;
}
Mysql_ROW::iterator Mysql_ROW::iterator::operator--( int)//前缀--
{
    Mysql_ROW::iterator i(this->iter--);
    return i;
}
bool operator==(Mysql_ROW::iterator i1, Mysql_ROW::iterator i2)
{
    return (i1.iter == i2.iter)?true:false;
}
bool operator!=(Mysql_ROW::iterator i1, Mysql_ROW::iterator i2)
{
    return (i1.iter != i2.iter)?true:false;
}

void Mysql_ROW::iterator::operator=(const Mysql_ROW::iterator & i)
{
    this->iter = i.iter;
}
Mysql_ROW::iterator::iterator()
{
}
Mysql_ROW::iterator::iterator( std::map<std::string, std::string>::iterator i)
{
    iter = i;
}
std::string Mysql_ROW::iterator::getName()
{
    return this->iter->first;
}
std::string Mysql_ROW::iterator::getValue()
{
    return this->iter->second;
}
Mysql_ROW::Mysql_ROW()
{
    lock();
    row  = new std::map<std::string , std::string>();
    unlock();
}
Mysql_ROW::~Mysql_ROW()
{
    lock();
    delete row;
    unlock();
}
Mysql_ROW::iterator Mysql_ROW::begin()
{
    lock();
    std::map<std::string , std::string>::iterator i = this->row->begin();
    unlock();
    return Mysql_ROW::iterator(i);
}
Mysql_ROW::iterator Mysql_ROW::end()
{
    lock();
    std::map<std::string , std::string>::iterator i = this->row->end();
    unlock();
    return Mysql_ROW::iterator(i);
}
Mysql_ROW::iterator Mysql_ROW::find(std::string name)
{
    lock();
    std::map<std::string , std::string>::iterator i = this->row->find(name);
    unlock();
    return Mysql_ROW::iterator(i);
}
void Mysql_ROW::insert(std::string name, std::string value)
{
    lock();
    row->insert(std::pair<std::string, std::string>(name, value));
    unlock();
}
void Mysql_ROW::erase(Mysql_ROW::iterator &i)
{
    lock();
    row->erase(i.iter);
    unlock();
}
void Mysql_ROW::erase(std::string name)
{
    lock();
    row->erase(row->find(name));
    unlock();
}
void Mysql_ROW::clear()
{
    lock();
    row->clear();
    unlock();
}
size_t Mysql_ROW::size()
{
    lock();
    size_t num = row->size();
    unlock();
    return  num;
}
Mysql_ROWS::Mysql_ROWS()
{
    lock();
    rows = new std::list<Mysql_ROW *>();
    unlock();
}
Mysql_ROWS::~Mysql_ROWS()
{
    lock();
    std::list<Mysql_ROW *>::iterator i=rows->begin();
    for(;i != rows->end() ;i++)
    {
        if(*i != NULL)
            delete *i;
    }
    delete rows;
    unlock();
}
Mysql_ROWS::iterator::iterator()
{
}
Mysql_ROWS::iterator::iterator(std::list<Mysql_ROW *>::iterator i)
{
    iter = i;
}
Mysql_ROWS::iterator Mysql_ROWS::iterator::operator++()//前缀++
{
    Mysql_ROWS::iterator i(++this->iter);
    return i;
}
Mysql_ROWS::iterator Mysql_ROWS::iterator::operator--()
{
    Mysql_ROWS::iterator i(--this->iter);
    return i;
}
Mysql_ROWS::iterator Mysql_ROWS::iterator::operator++(int)//后缀++
{
    Mysql_ROWS::iterator i(this->iter++);
    return i;
}
Mysql_ROWS::iterator Mysql_ROWS::iterator::operator--(int)
{
    Mysql_ROWS::iterator i(this->iter--);
    return i;
}
int Mysql_ROWS::iterator::operator==(Mysql_ROWS::iterator i)
{
    return (this->iter == i.iter)?1:0;
}
int Mysql_ROWS::iterator::operator!=(Mysql_ROWS::iterator i)
{
    return (this->iter != i.iter)?1:0;
}

void Mysql_ROWS::iterator::operator=(const Mysql_ROWS::iterator & i)
{
    this->iter = i.iter;
}
Mysql_ROW * Mysql_ROWS::iterator::operator*()
{
    return *iter;
}
void Mysql_ROWS::pop_back()
{
    lock();
    rows->pop_back();
    unlock();
}
void Mysql_ROWS::pop_front()
{
    lock();
    rows->pop_front();
    unlock();
}
void Mysql_ROWS::push_back(Mysql_ROW * row)
{
    lock();
    rows->push_back(row);
    unlock();
}
void Mysql_ROWS::push_front(Mysql_ROW * row)
{
    lock();
    rows->push_front(row);
    unlock();
}
Mysql_ROW * Mysql_ROWS::back()
{
    lock();
    Mysql_ROW * row = rows->back();
    unlock();
    return row;
}
Mysql_ROW * Mysql_ROWS::front()
{
    lock();
    Mysql_ROW * row = rows->front();
    unlock();
    return row;
}
Mysql_ROWS::iterator Mysql_ROWS::begin()
{
    lock();
    std::list<Mysql_ROW *>::iterator i = rows->begin();
    unlock();
    return Mysql_ROWS::iterator(i);
}
Mysql_ROWS::iterator Mysql_ROWS::end()
{
    lock();
    std::list<Mysql_ROW *>::iterator i = rows->end();
    unlock();
    return Mysql_ROWS::iterator(i);
}
void Mysql_ROWS::erase(Mysql_ROWS::iterator &i)
{
    lock();
    rows->erase(i.iter);
    unlock();
}
void Mysql_ROWS::insert(Mysql_ROWS::iterator &i,  Mysql_ROW * row)
{
    lock();
    rows->insert(i.iter , row);
    unlock();
}
void Mysql_ROWS::clear()
{
    lock();
    rows->clear();
    unlock();
}
size_t Mysql_ROWS::size()
{
    lock();
    size_t num = rows->size();
    unlock();
    return  num;
}
Mysql_Res::iterator Mysql_Res::iterator::operator++()//前缀++
{
    Mysql_Res::iterator i(++this->iter);
    return i;
}
Mysql_Res::iterator Mysql_Res::iterator::operator++( int)//后缀++
{
    Mysql_Res::iterator i(this->iter++);
    return i;
}
Mysql_Res::iterator Mysql_Res::iterator::operator--()//前缀--
{
    Mysql_Res::iterator i(--this->iter);
    return i;
}
Mysql_Res::iterator Mysql_Res::iterator::operator--( int)//前缀--
{
    Mysql_Res::iterator i(this->iter--);
    return i;
}
bool operator==(Mysql_Res::iterator i1, Mysql_Res::iterator i2)
{
    return (i1.iter == i2.iter)?true:false;
}
bool operator!=(Mysql_Res::iterator i1, Mysql_Res::iterator i2)
{
    return (i1.iter != i2.iter)?true:false;
}

void Mysql_Res::iterator::operator=(const Mysql_Res::iterator & i)
{
    this->iter = i.iter;
}
Mysql_Res::iterator::iterator()
{
}
Mysql_Res::iterator::iterator( std::map<std::string, Mysql_strList *>::iterator i)
{
    iter = i;
}
std::string Mysql_Res::iterator::getName()
{
    return this->iter->first;
}
Mysql_strList * Mysql_Res::iterator::getValue()
{
    return this->iter->second;
}
Mysql_Res::Mysql_Res()
{
    lock();
    rows  = new std::map<std::string , Mysql_strList *>();
    unlock();
}
Mysql_Res::~Mysql_Res()
{
    lock();
    std::map<std::string , Mysql_strList *>::iterator i = this->rows->begin();
    for(;i != this->rows->end();i++)
    {
        if(i->second != NULL)
            delete i->second;
    }
    delete rows;
}
Mysql_Res::iterator Mysql_Res::begin()
{
    lock();
    std::map<std::string , Mysql_strList *>::iterator i = this->rows->begin();
    unlock();
    return Mysql_Res::iterator(i);
}
Mysql_Res::iterator Mysql_Res::end()
{
    lock();
    std::map<std::string ,Mysql_strList *>::iterator i = this->rows->end();
    unlock();
    return Mysql_Res::iterator(i);
}
Mysql_Res::iterator Mysql_Res::find(std::string name)
{
    lock();
    std::map<std::string , Mysql_strList *>::iterator i = this->rows->find(name);
    unlock();
    return Mysql_Res::iterator(i);
}
void Mysql_Res::insert(std::string name, Mysql_strList * value)
{
    lock();
    rows->insert(std::pair<std::string, Mysql_strList *>(name, value));
    unlock();
}
void Mysql_Res::erase(Mysql_Res::iterator &i)
{
    lock();
    rows->erase(i.iter);
    unlock();
}
void Mysql_Res::erase(std::string name)
{
    lock();
    rows->erase(rows->find(name));
    unlock();
}
void Mysql_Res::clear()
{
    lock();
    rows->clear();
    unlock();
}
size_t Mysql_Res::size()
{
    lock();
    size_t num = rows->size();
    unlock();
    return  num;
}
