/*************************************************************************
# > File Name: HttpApiServer.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-09 18:04:49
# > Revise Time: 2018-10-17 12:19:16
 ************************************************************************/

#include<iostream>
#include "HttpApiServer.h"
#include <event2/util.h>
#include <event2/event_struct.h>
#include "Http_websocket/include/config.h"
#include "Http_websocket/include/config.h"
#include "CJsonObject/CJsonObject.hpp"
#define APICLIENT_TIMEOUT 1800//30分钟
#define APICLIENT_TIMEOUT_CHECK_PERIOD 1000//1S
const char *getFileType(const char *filename);
int getFileSize(const char * filename);
void serve_file(struct httpChilent_t *  client, const char *filename);
void getRandomString(char *buf, int len)
{
    int i;
    char temp;
    srand((int)time(0));
    i = 0;
    while(i < len)
    {
        temp = (rand()%3);
        if(temp == 0)
        {
            temp = (rand()%('Z' - 'A' + 1) + 'A');
        }else if(temp == 1)
        {
            temp = (rand()%('z' - 'a' + 1) + 'a');
        }
        else
        {
            temp = (rand()%('9' - '0' + 1) + '0');
        }
        if((temp > 'A' && temp < 'Z') || (temp > 'a' && temp < 'z') || (temp > '0' && temp < '9'))    // 随机数不要0, 0 会干扰对字符串长度的判断
        {
            buf[i] = temp;
            i++;
        }
    }
}
void HttpApiServer::UserListClass::insert(std::string user, std::string key)
{
    lock();
    keyList.insert(std::pair<std::string,std::string>(key, user));
    userList.insert(std::pair<std::string,std::string>(user, key));
    struct timeval tv;
    gettimeofday(&tv,NULL);
    LatelyOperatorTimeList.insert(std::pair<std::string,timeval>(user, tv));
    unlock();
}
bool HttpApiServer::UserListClass::reinsert(std::string user,std::string newKey)
{
    if(user.empty() || newKey.empty())
        return false;
    lock();
    auto i = userList.find(user);
    if(i != userList.end())
    {
        auto j =  keyList.find(i->second);
        auto x = LatelyOperatorTimeList.find(user);
        keyList.erase(j);
        LatelyOperatorTimeList.erase(x);
        userList.erase(i);
    }
    unlock();
    insert(user, newKey);
    return true;
}
bool HttpApiServer::UserListClass::erase(std::string key, std::string user)
{
    lock();
    if(!key.empty())
    {
        auto i = keyList.find(key);
        if(i != keyList.end())
        {
            auto j =  userList.find(i->second);
            auto x = LatelyOperatorTimeList.find(i->second);
            LatelyOperatorTimeList.erase(x);
            userList.erase(j);
            keyList.erase(i);
            unlock();
            return true;
        }
    }
    else if(!user.empty())
    {
        auto i = userList.find(user);
        if(i != userList.end())
        {
            auto j =  keyList.find(i->second);
            auto x = LatelyOperatorTimeList.find(user);
            LatelyOperatorTimeList.erase(x);
            keyList.erase(j);
            userList.erase(i);
            unlock();
            return true;
        }
    }
    unlock();
    return false;
}
std::string HttpApiServer::UserListClass::findUser(std::string key)
{
    if(key.empty())
        return "";
    lock();
    auto i = keyList.find(key);
    if(i != keyList.end())
    {
        std::string user = i->second;
        gettimeofday(&LatelyOperatorTimeList.find(user)->second,NULL);
        unlock();
        return user;
    }
    unlock();
    return "";
}
std::string HttpApiServer::UserListClass::findKey(std::string user)
{
    if(user.empty())
        return "";
    lock();
    auto i = userList.find(user);
    if(i != userList.end())
    {
        gettimeofday(&LatelyOperatorTimeList.find(user)->second,NULL);
        std::string key = i->second;
        unlock();
        return key;
    }
    unlock();
    return "";
}

void HttpApiServer::Resqonse(struct httpChilent_t *httpChilent, std::string out)
{
    char num[20];
    sprintf(num,"%d",(int)out.size());
    httpChilent->resqonse.resqonseCode = HTTP_OK;
    httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type","application/json"));
    httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
    httpChilent->resqonse.resqonse_data = (char *)malloc(out.size()+1);
    strcpy(httpChilent->resqonse.resqonse_data, out.c_str());
    httpChilent->resqonse.Resqonse_data_len = out.size();

}
void HttpApiServer::ResqonseError(struct httpChilent_t *httpChilent,int error_V)
{
    char num[20];
    httpChilent->resqonse.resqonseCode = error_V;
    neb::CJsonObject json = neb::CJsonObject();
    json.Add("stat",-1);
    std::string out =  json.ToString();
    sprintf(num,"%d",(int)out.size());
    httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
    httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type","application/json"));
    httpChilent->resqonse.resqonse_data = (char *)malloc(out.size()+1);
    strcpy(httpChilent->resqonse.resqonse_data, out.c_str());
    httpChilent->resqonse.Resqonse_data_len = out.size();
}
void HttpApiServer::login(struct httpChilent_t * httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);
    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);
        return;
    }
    std::string user;
    std::string pwd;
    json.Get("user", user);
    json.Get("pwd", pwd);
    int rec = -3;
    try
    {
        if(DataBase::getInstance().CheckUpUser(user) == 0)
        {
            rec = DataBase::getInstance().UserLogin(user,pwd);
            if(rec != 0)
                rec = -2;
        }
        else
            rec = -1;
    }
    catch(std::string err_str)
    {
        printf("%s\r\n", err_str.c_str());
        rec = -3;
    }
    std::string out;
    if(rec == 0)
    {
        neb::CJsonObject json = neb::CJsonObject();
        json.Add("user",user);
        json.Add("stat",0);
        char key[17];
        key[16] = '\0';
        getRandomString(key,16);
        while(!userList.findUser(key).empty())
        {
            getRandomString(key,16);
        }

        json.Add("key",(char *)key);
        out = json.ToString();
        userList.reinsert(user, key);    
        httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Cookie",key));
    }
    else if(rec == -1)
    {
        neb::CJsonObject json = neb::CJsonObject();
        json.Add("user",user);
        json.Add("stat",2);
        out = json.ToString();
    }
    else if(rec == -2)
    {
        neb::CJsonObject json = neb::CJsonObject();
        json.Add("user",user);
        json.Add("stat",1);
        out = json.ToString();
    }
    Resqonse(httpChilent, out);    

}
void HttpApiServer::Register(struct httpChilent_t *httpChilent)
{ 
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);
    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string user;
    std::string pwd;
    std::string Email;
    std::string nikename;
    json.Get("user",user);
    json.Get("pwd",pwd);
    json.Get("Email", Email);
    json.Get("nikename", nikename);
    int rec = -2;
    if(!user.empty() && !pwd.empty() && !nikename.empty())
    {
        try
        {
            if(DataBase::getInstance().CheckUpUser(user) != 0)
            {
                if(Email.empty())
                {
                    if(DataBase::getInstance().AddUser(user, pwd, nikename) == 0)
                    {
                        rec = 0;
                    }
                }
                else
                {
                    if(DataBase::getInstance().AddUser(user, pwd, nikename, Email) == 0)
                    {
                        rec = 0;
                    }

                }
            }
            else
                rec = -1;
        }
        catch(std::string err_str)
        {
            printf("%s\r\n", err_str.c_str());
        }
    }
    neb::CJsonObject json_out = neb::CJsonObject();
    if(rec == 0)
    {
        json_out.Add("stat",0);
        json_out.Add("user",user);
    }
    else if(rec == -1)
    {
        json_out.Add("stat",1);
        json_out.Add("user",user);
    }
    else if(rec == -2)
    {
        json_out.Add("stat",2);
        json_out.Add("user",user);
    }
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out);    
}
void HttpApiServer::logon(struct httpChilent_t * httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);
    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string key;
    neb::CJsonObject json_out;
    json.Get("key",key); 
    std::string user = userList.findUser(key);
    if(userList.erase(key) == true)
    {

        json_out.Add("user",user);
        json_out.Add("stat",0);
    }
    else
    {
        json_out.Add("stat",1);
    }
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out);    
}
void HttpApiServer::AddSensing(struct httpChilent_t *httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);

    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string key; 
    std::string IdCode;
    std::string Secretkey;
    json.Get("key", key);
    json.Get("IdCode", IdCode);
    json.Get("Secretkey", Secretkey);
    neb::CJsonObject json_out;
    int rec = -2;
    std::string user;
    if(!key.empty() && !IdCode.empty() && !Secretkey.empty())
    {
        user = userList.findUser(key);
        if(!user.empty())
        {
            try
            {
                if(DataBase::getInstance().CheckUpSensor(IdCode).empty())
                {
                    if(DataBase::getInstance().AddSensor(user, IdCode, Secretkey) == 0)
                    {
                        rec = 0;
                    }
                }
                else
                    rec = -1;
            }
            catch(std::string err_str)
            {
                printf("%s\r\n", err_str.c_str());
            }
        }
    }
    if(rec == 0)
    {
        json_out.Add("user",user);
        json_out.Add("stat",0);
    }
    else if(rec == -1)
    {
        json_out.Add("user",user);
        json_out.Add("stat",1);
    }
    else if(rec == -2)
    {
        if(!user.empty())
            json_out.Add("user",user);
        json_out.Add("stat",2);
    }
    if(!user.empty())
        httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Cookie",key));
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out);    
}
void HttpApiServer::DelSensing(struct httpChilent_t *httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);

    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string key; 
    std::string IdCode;
    json.Get("key", key);
    json.Get("IdCode", IdCode);
    neb::CJsonObject json_out;
    int rec = -3;
    std::string user;
    if(!key.empty() && !IdCode.empty())
    {
        user = userList.findUser(key);
        if(!user.empty())
        {
            try
            {
                if(!DataBase::getInstance().CheckUpSensor(IdCode).empty())
                {
                    if(DataBase::getInstance().CheckUpSensorIsUser(user, IdCode) == 0)
                    {
                        if(DataBase::getInstance().DelSensor(user, IdCode) == 0)
                        {
                            rec = 0;
                        }
                    }
                    else
                    {
                        rec = -2;
                    }
                    printf("%s %s", user.c_str(), IdCode.c_str());
                }
                else
                    rec = -1;
            }
            catch(std::string err_str)
            {
                printf("%s\r\n", err_str.c_str());
            }
        }
    }
    if(rec == 0)
    {
        json_out.Add("user",user);
        json_out.Add("IdCode",IdCode);
        json_out.Add("stat",0);
    }
    else if(rec == -1)
    {
        json_out.Add("user",user);
        json_out.Add("IdCode",IdCode);
        json_out.Add("stat",1);
    }
    else if(rec == -2)
    {
        json_out.Add("user",user);
        json_out.Add("IdCode",IdCode);
        json_out.Add("stat",2);
    }
    else if(rec == -3)
    {
        if(!user.empty())
            json_out.Add("user",user);
        json_out.Add("IdCode",IdCode);
        json_out.Add("stat",3);
    }
    if(!user.empty())
        httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Cookie",key));
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out);    
}

void HttpApiServer::GetSensingList(struct httpChilent_t *httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);

    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string key; 
    json.Get("key", key);
    neb::CJsonObject json_out;
    int rec = -2;
    std::string user;
    do
    {
        if(!key.empty())
        {
            user = userList.findUser(key);
            if(!user.empty())
            {
                try
                {
                    Mysql_strList * rec_list =  DataBase::getInstance().GetUserSensorList(user);
                    if(rec_list == NULL)
                    {
                        rec = -1;
                        break;
                    }
                    json_out.Add("Num",(int)rec_list->size());
                    json_out.AddEmptySubArray("List");
                    for(Mysql_strList::iterator i = rec_list->begin();i != rec_list->end();i++)
                    {
                        neb::CJsonObject json_sensing;
                        json_sensing.Add("IdCode",*i);
                        std::string Secretkey = DataBase::getInstance().CheckUpSensor(*i);
                        if(Secretkey.empty())
                            throw  "Secretkey null";
                        json_sensing.Add("Secretkey",Secretkey);
                        json_sensing.Add("Stat",1);
                        json_sensing.Add("Ip","192.168.1.1");
                        json_sensing.Add("Port",8088);
                        json_out["List"].Add(json_sensing);
                    }
                    delete rec_list;
                }
                catch(std::string err_str)
                {
                    printf("%s\r\n", err_str.c_str());
                }
            }
        }
    }while(0);
    if(rec == 0)
    {
        json_out.Add("user",user);
        json_out.Add("stat",0);
    }
    else if(rec == -1)
    {
        json_out.Add("user",user);
        json_out.Add("stat",1);
    }
    else if(rec == -2)
    {
        if(!user.empty())
            json_out.Add("user",user);
        json_out.Add("stat",2);
    }
    if(!user.empty())
        httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Cookie",key));
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out); 

}
void HttpApiServer::GetSensingData(struct httpChilent_t *httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);

    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string key; 
    std::string IdCode;
    std::string startDate; 
    std::string endDate; 
    json.Get("key", key);
    json.Get("IdCode", IdCode);
    json["Date"].Get(0,startDate);
    json["Date"].Get(1,endDate);
    neb::CJsonObject json_out;
    int rec = -3;
    std::string user;
    do
    {
        if(!key.empty() && !IdCode.empty() && !startDate.empty() && !endDate.empty())
        {
            user = userList.findUser(key);
            if(!user.empty())
            {
                try
                {
                    Mysql_Res * res;
                    if(strcmp(IdCode.c_str(), "0") == 0)
                        goto s1;
                    if(!DataBase::getInstance().CheckUpSensor(IdCode).empty())
                    {
                        if(DataBase::getInstance().CheckUpSensorIsUser(user, IdCode) == 0)
                        {

                            if(0)
s1:                             res = DataBase::getInstance().GetUserAllSensorRes(user,startDate, endDate);
                            else
                                res =  DataBase::getInstance().GetSensorRes(IdCode, startDate, endDate);
                            if(res == NULL)
                            {
                                break;
                            }
                            json_out.Add("Num",(int)res->size());
                            json_out.AddEmptySubArray("Data");
                            for(Mysql_Res::iterator i = res->begin();i != res->end();i++)
                            {
                                if(i == res->begin())
                                {
                                    for(Mysql_strList::iterator j = i.getValue()->begin();j != i.getValue()->end();j++)
                                    {
                                        neb::CJsonObject json_objrect;
                                        json_objrect.Add(i.getName(),*j);
                                        json_out["Data"].Add(json_objrect);
                                    }

                                }
                                else
                                {
                                    int x = 0;
                                    for(Mysql_strList::iterator j = i.getValue()->begin();j != i.getValue()->end();j++, x++)
                                    {
                                        json_out["Data"][x].Add(i.getName(),*j);
                                    }


                                }
                                
                            }
                            delete res;
                            rec = 0;
                        }
                        else
                            rec = -2;
                    }
                    else
                        rec= -1;

                }
                catch(std::string err_str)
                {
                    printf("%s\r\n", err_str.c_str());
                }
            }
        }
    }while(0);
    if(rec == 0)
    {
        json_out.Add("user",user);
        json_out.Add("stat",0);
    }
    else if(rec == -1)
    {
        json_out.Add("user",user);
        json_out.Add("stat",1);
    }
    else if(rec == -2)
    {
        json_out.Add("user",user);
        json_out.Add("stat",2);
    }
    else if(rec == -3)
    {
        if(!user.empty())
            json_out.Add("user",user);
        json_out.Add("stat",3);
    }
    if(!user.empty())
        httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Cookie",key));
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out); 
}
void HttpApiServer::GetConSensing(struct httpChilent_t *httpChilent)
{
    httpChilent->request.request_data = (char *)realloc(httpChilent->request.request_data, httpChilent->request.request_data_len + 1);
    if(httpChilent->request.request_data == NULL)
    {
        ResqonseError(httpChilent, HTTP_INTERNAL);    
        return;
    }
    httpChilent->request.request_data[httpChilent->request.request_data_len] = 0;
    neb::CJsonObject json = neb::CJsonObject(httpChilent->request.request_data);

    if(json.IsEmpty())
    {
        ResqonseError(httpChilent, HTTP_BADREQUEST);    
        return;
    }
    std::string key; 
    std::string IdCode;
    json.Get("key", key);
    json.Get("IdCode", IdCode);
    neb::CJsonObject json_out;
    int rec = -1;
    std::string user;
    if(!key.empty() && !IdCode.empty())
    {
        user = userList.findUser(key);
        if(!user.empty())
        {
            try
            {
                if(!DataBase::getInstance().CheckUpSensor(IdCode).empty())
                {
                    if(DataBase::getInstance().CheckUpSensorIsUser(user,IdCode) == 0)
                        rec = 0;
                    else
                        rec = -4;
                }
                else
                    rec = -2;
            }
            catch(std::string err_str)
            {
                printf("%s\r\n", err_str.c_str());
            }
        }
    }
    if(rec == 0)
    {
        json_out.Add("user",user);
        json_out.Add("stat",0);
    }
    else if(rec == -1)
    {
        if(!user.empty())
            json_out.Add("user",user);
        json_out.Add("stat",1);
    }
    else if(rec == -2)
    {
        json_out.Add("user",user);
        json_out.Add("stat",2);
    }
    else if(rec == -3)
    {
        json_out.Add("user",user);
        json_out.Add("stat",3);
    }
    else if(rec == -4)
    {
        json_out.Add("user",user);
        json_out.Add("stat",4);
    }
    if(!user.empty())
        httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Cookie",key));
    std::string out;
    out = json_out.ToString();
    Resqonse(httpChilent, out); 
}
void *HttpApiServer_httplisten_thread(void * arg)
{
    HttpApiServer * httpApiServer = (HttpApiServer *)arg;
    event_base_dispatch(httpApiServer->listen_base);
    httpApiServer->lock();
    httpServer_free(httpApiServer->httpServer);
    event_base_free(httpApiServer->listen_base);
    httpApiServer->listen_base = NULL;
    httpApiServer->httpServer = NULL;
    httpApiServer->listen_th = 0;
    httpApiServer->unlock();
    return NULL;
}
void ev_time_inspect_cb(evutil_socket_t fd, short events, void *arg)
{
    fd = (evutil_socket_t)fd;
    events = (short)events;
    HttpApiServer * httpApiServer = (HttpApiServer *)arg;
    httpApiServer->lock();
    if(httpApiServer->listen_base == NULL || httpApiServer->httpServer == NULL)
    {
        httpApiServer->error.IsException = true;
        httpApiServer->error.errorType = HttpApiServerException::ErrorType::DEADLY_ERROE;

        httpApiServer->error.err_str = "listen_base end!";
    }
    if(httpApiServer->error.IsException == true)
    {
        httpApiServer->end_loop();
    }
    httpApiServer->unlock();
    httpApiServer->userList.lock();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    for(auto i = httpApiServer->userList.LatelyOperatorTimeList.begin();
        i != httpApiServer->userList.LatelyOperatorTimeList.end();)
    {
        if(tv.tv_sec -  i->second.tv_sec >= APICLIENT_TIMEOUT )
        {
            auto j =  httpApiServer->userList.userList.find(i->first);
            auto x =  httpApiServer->userList.keyList.find(i->first);
            httpApiServer->userList.keyList.erase(x);
            httpApiServer->userList.userList.erase(j);
            httpApiServer->userList.LatelyOperatorTimeList.erase(i++);
        }
        else
            i++;
    }
    httpApiServer->userList.unlock();

}
void ws_read_cb(struct httpChilent_t * httpChilent)
{
    HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(httpChilent->httpServer);
    char buff[1024];
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in *address = (struct sockaddr_in *)&(httpChilent->addr);
    inet_ntop(AF_INET, &address->sin_addr,ip, INET_ADDRSTRLEN);
    evbuffer_remove(httpChilent->receive_evbuff, buff, evbuffer_get_length(httpChilent->receive_evbuff));
}
void httpHandler_cb(struct httpChilent_t * httpChilent)
{
    HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(httpChilent->httpServer);
    char path[512];
    do{
        if(strcasecmp(httpChilent->request.method, "GET") == 0)
        {
            if(strcmp(httpChilent->request.url,"/") == 0 || strcmp(httpChilent->request.url, "/null") == 0)
            {
                sprintf(path, "./wwwroot/index.html");
            }
            else
            {
                sprintf(path, "./wwwroot%s", httpChilent->request.url);
                if(httpChilent->request.url[strlen(httpChilent->request.url) - 1] == '/')
                {
                    strcat(path, "index.html");
                }

            }
            serve_file(httpChilent, path);

        }
        else if(strcasecmp(httpChilent->request.method, "POST") == 0)
        {
            if(httpChilent->request.request_data != NULL)
            {
                if(strcmp(httpChilent->request.url, "/login") == 0)     
                {
                    httpApiServer->login(httpChilent);
                }
                else if(strcmp(httpChilent->request.url, "/logon") == 0)
                {
                    httpApiServer->logon(httpChilent);
                }
                else if(strcmp(httpChilent->request.url,"/register") == 0)
                {
                    httpApiServer->Register(httpChilent);
                }
                else if(strcmp(httpChilent->request.url,"/add-sensing") == 0)
                {
                    httpApiServer->AddSensing(httpChilent);
                }
                else if(strcmp(httpChilent->request.url,"/del-sensing") == 0)
                {
                    httpApiServer->DelSensing(httpChilent);
                }
                else if(strcmp(httpChilent->request.url,"/get-sensingList") == 0)
                {
                    httpApiServer->GetSensingList(httpChilent);
                }
                else if(strcmp(httpChilent->request.url,"/get-sensingData") == 0)
                {
                    httpApiServer->GetSensingData(httpChilent);
                }
                else if(strcmp(httpChilent->request.url,"/get-conSensing") == 0)
                {
                    httpApiServer->GetConSensing(httpChilent);
                }
                else
                {
                    httpChilent->resqonse.resqonseCode = 404;
                }
            }
        }
        if(httpChilent->request.HeadParameter != NULL)
        {
            HeadParameterMap::iterator v;
            v = httpChilent->request.HeadParameter->find("Connection");
            if(v != httpChilent->request.HeadParameter->end())
            {
                if(strcasecmp(v->second.c_str(), "keep-alive") == 0)
                {

                }
                else if(strcasecmp(v->second.c_str(), "close") == 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        resqonse(httpChilent);
        return;
    }while(0);
    resqonse(httpChilent);
    httpChilent_Close(httpChilent);

}
HttpApiServer::HttpApiServer(u_short port)
{
    do
    {
        if(port == 0)
        {
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::COMMONLY_ERROR;
            error.err_str = "http_port == 0";
            break;
        }
        http_port = port;
        try
        {
            DataBase::getInstance();//初始化数据层
        }
        catch(std::string err)
        {
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::DEADLY_ERROE;
            error.err_str = err;
            break;
        }
        listen_base = event_base_new(); 
        if(listen_base == NULL)
        {
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::COMMONLY_ERROR;
            error.err_str = "event_base_new fail!";
            break;
        }
        main_base = event_base_new();
        if(main_base == NULL)
        {
            event_base_free(this->listen_base);
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::COMMONLY_ERROR;
            error.err_str = "event_base_new fail!";
            break;
        }
        set_SERVER_STRING(NULL);
        httpServer = startHttpServe(this->listen_base, &http_port);
        if(httpServer == NULL)
        {
            event_base_free(this->listen_base);
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::COMMONLY_ERROR;
            error.err_str = "startHttpServe fail!";
            break;
        }
        httpServer_setArg(httpServer, this);
        httpServer_setHttpHandler(httpServer, httpHandler_cb);
        httpServer_setWebSocket_read_cb(httpServer, ws_read_cb, "/connect-sensing");

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int rec = pthread_create(&this->listen_th, &attr,HttpApiServer_httplisten_thread,(void *)this);
        pthread_attr_destroy(&attr);
        if(rec != 0)
        {
            httpServer_free(this->httpServer);
            event_base_free(this->listen_base);
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::COMMONLY_ERROR;
            error.err_str = "listen_th thread create fail!";
            break;
        }
        /*检查定时器 */
        event_assign(&ev_time_inspect, main_base, -1, EV_PERSIST, ev_time_inspect_cb, this);
        struct timeval tv;
        evutil_timerclear(&tv);
        tv.tv_sec = 0;
        tv.tv_usec = 1000*APICLIENT_TIMEOUT_CHECK_PERIOD; //200ms
        event_add(&ev_time_inspect, &tv);

    }while(0);
    if(error.IsException == true)
        throw error;
}
HttpApiServer::~HttpApiServer()
{
    unlock();
    if(httpServer != NULL)
    {
        httpServer_free(httpServer);
    }
    if(listen_base != 0)
    {
        event_base_loopexit(listen_base, NULL);
        if(listen_th !=0 )
        {
            unlock();
            pthread_join(listen_th, NULL);
            lock();
        }
    }
    if(main_base != NULL)
    {
        event_base_free(main_base);
    }
}
void HttpApiServer::start_loop()
{
    event_base_dispatch(this->main_base);
    lock();
    if(error.IsException == true)
    {
        unlock();
        throw error;
    }
    unlock();
}
void HttpApiServer::end_loop()
{
    event_base_loopexit(main_base, NULL);
}
HttpApiServerException::HttpApiServerException()
{
    IsException = false;
}
std::string HttpApiServerException::toString()
{
    return err_str;
}
const char *getFileType(const char *filename){ //根据扩展名返回文件类型描述  
    char sExt[32];
    const char *p_start = filename + strlen(filename) - 1;
    memset(sExt, 0, sizeof(sExt));
    while (*p_start)
    {
        if (*p_start == '.')
        {
            p_start++;
            strncpy(sExt, p_start, sizeof(sExt));
            break;

        }
        p_start--;

    }

    if (strncmp(sExt, "bmp", 3) == 0)
        return "image/bmp";

    if (strncmp(sExt, "gif", 3) == 0)
        return "image/gif";

    if (strncmp(sExt, "ico", 3) == 0)
        return "image/x-icon";

    if (strncmp(sExt, "jpg", 3) == 0)
        return "image/jpeg";

    if (strncmp(sExt, "avi", 3) == 0)
        return "video/avi";

    if (strncmp(sExt, "css", 3) == 0)
        return "text/css";

    if (strncmp(sExt, "dll", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "exe", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "dtd", 3) == 0)
        return "text/xml";

    if (strncmp(sExt, "mp3", 3) == 0)
        return "audio/mp3";

    if (strncmp(sExt, "mpg", 3) == 0)
        return "video/mpg";

    if (strncmp(sExt, "png", 3) == 0)
    {
        return "image/png";
    }

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/vnd.ms-powerpoint";

    if (strncmp(sExt, "xls", 3) == 0)
        return "application/vnd.ms-excel";

    if (strncmp(sExt, "doc", 3) == 0)
        return "application/msword";

    if (strncmp(sExt, "mp4", 3) == 0)
        return "video/mpeg4";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/x-ppt";

    if (strncmp(sExt, "wma", 3) == 0)
        return "audio/x-ms-wma";

    if (strncmp(sExt, "wmv", 3) == 0)
        return "video/x-ms-wmv";
    if(strncmp(sExt, "wmv", 3) == 0)
        return "application/x-javascript";
    return "text/html";
}
int getFileSize(const char * filename)
{
    int fileSize = 0;
    FILE *fp = NULL;
    if (filename == NULL)
        return 0;
    //以二进制的形式读取
    fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    fclose(fp);
    return fileSize;
}
void serve_file(struct httpChilent_t *  client, const char *filename)
{
    int fp = open(filename,O_RDONLY|O_NONBLOCK);
    char num[20];
    if (fp == -1)
    {
        client->resqonse.resqonseCode = HTTP_NOTFOUND;
    }
    else
    {
        int len = getFileSize(filename);
        if(len == 0)
        {
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
        sprintf(num,"%d",len);
        client->resqonse.Resqonse_data_len = len;
        client->resqonse.resqonse_data = (char *)malloc(client->resqonse.Resqonse_data_len);
        if(client->resqonse.resqonse_data == NULL)
        {
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
        client->resqonse.resqonseCode = HTTP_OK;
        client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type", getFileType(filename)));
        client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
        int rec;
        int read_len = 0;
        while((rec = read(fp, client->resqonse.resqonse_data + read_len, 1024)) >0)
        {
            pthread_mutex_lock(&client->lock);
            gettimeofday(&client->final_optime,NULL);
            pthread_mutex_unlock(&client->lock);
            read_len += rec;
        }
        if(rec == -1)
        {
            perror("rev");
            free(client->resqonse.resqonse_data);
            client->resqonse.resqonse_data = NULL;
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
    }
    close(fp);
}



