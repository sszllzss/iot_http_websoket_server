/*************************************************************************
# > File Name: HttpApiServer.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-09 18:04:49
# > Revise Time: 2019-01-04 11:13:39
 ************************************************************************/

#include<iostream>
#include "HttpApiServer.h"
#include <event2/util.h>
#include <event2/event_struct.h>
#include "Http_websocket/include/config.h"
#include "Http_websocket/include/config.h"
#include "Http_websocket/include/debug.h"
#define APICLIENT_TIMEOUT 1800//30分钟
#define APICLIENT_TIMEOUT_CHECK_PERIOD 1000//1S
#define APICLIENT_CMD_LOSS_MAX_NUM 10
#define APICLIENT_HEARBEAT_SENSING_TIME_OUT 36000 //10min 心跳超时 
#define APICLIENT_SENSING_CHECH_TIME 2 //2s 定时器检测时间
#define APICLIENT_WS_CHECH_TIME 2 //2s 定时器检测时间
const char *getFileType(const char *filename);
int getFileSize(const char * filename);
void serve_file(struct httpChilent_t *  client, const char *filename);
void WebSocketOperating::ComReply(neb::CJsonObject &req,neb::CJsonObject &res)
{
    printf("测试 %s\r\n", req.ToString().c_str());
    printf("测试 %s\r\n", res.ToString().c_str());
    
    
}
void WebSocketOperating::ComReplyNo(neb::CJsonObject &req)
{
    printf("测试 %s\r\n", req.ToString().c_str());
}
void WebSocketOperating::ComExe(neb::CJsonObject &json)
{
    std::string type;
    neb::CJsonObject dataJson;
    int id = -1;
    json.Get("type", type);
    json.Get("Data", dataJson);
    json.Get("ID", id);
    if(Authentication == true)
    {
        if(strcmp(type.c_str(), "StatDat") == 0)
        {
            std::string cmd;
            dataJson.Get("cmd",cmd);
            if(strcmp(cmd.c_str(), "CarTest") == 0)
                CarTest(id);
        }
        else if(strcmp(type.c_str(), "UartData") == 0)
        {

        }
    }
    else
    {
        if(strcmp(type.c_str(), "Authentication") == 0)
        {
            std::string key;
            dataJson.Get("key",key);
            if(id == 0 && !key.empty())
            {
                authentication(key);        
                return;
            }
        }
        HttpServer_webSocket_close(this->htppChilent);
    }

    
}
void  WebSocketOperating::CarTest(int id)
{
    SensingOperating * sensingOperating =  this->httpApiServer->userList.User_Sensing_MapfindSensing(this->User);
    if(sensingOperating != NULL)
    {
        neb::CJsonObject json_out;
        neb::CJsonObject json_data;
        json_data.Add("cmd","heartbeat");
        json_out.Add("res_req","req");
        json_out.Add("type","cmd");
        json_out.Add("Data", json_data);
        this->idQueue_push(id);
        sensingOperating->sendCmd(json_out);
    }

}
void WebSocketOperating::authentication(std::string key)
{
    std::string user =  this->httpApiServer->userList.findUser(key);
    if(!user.empty())
    {
        if(this->httpApiServer->userList.User_Sensing_MapfindSensing(user) != NULL)
        {
            this->httpApiServer->userList.AddSensing_User_Map_client(user , this->htppChilent);
            this->Authentication = true;
            this->User = user;
            neb::CJsonObject json_out;
            neb::CJsonObject json_data;
            json_data.Add("stat", 0);
            json_out.Add("ResReq","Res");
            json_out.Add("type","Authentication");
            json_out.Add("Data",json_data);
            this->sendCmd(json_out, 0);
            return;
        }
    }
    HttpServer_webSocket_close(this->htppChilent);
}


void SensingOperating::ComReply(neb::CJsonObject &req,neb::CJsonObject &res)
{
    std::string user =  this->httpApiServer->userList.User_Sensing_MapfindUser(this);
    httpChilent_t * httpChilent = this->httpApiServer->userList.User_Sensing_MapfindWebSocket(user);
    WebSocketOperating * webSocketOperating = this->httpApiServer->webSocketOperatingList->find(httpChilent);
    if(webSocketOperating != NULL)
    {

    }
    std::string type;
    :
    if()
    {

    }
    printf("测试 %s\r\n", req.ToString().c_str());
    printf("测试 %s\r\n", res.ToString().c_str());
    neb::CJsonObject json_out;
    neb::CJsonObject json_data;
    json_data.Add("cmd","CarTest");
    json_out.Add("ResReq","Res");
    json_out.Add("type","StatDat");
    json_out.Add("Data", json_data);
    this->sendCmd(json_out, id);
}
void SensingOperating::ComReplyNo(neb::CJsonObject &req)
{
    printf("测试 %s\r\n", req.ToString().c_str());
}
void SensingOperating::ComExe(neb::CJsonObject &json)
{
    std::string type;
    neb::CJsonObject dataJson;
    int id = -1;
    json.Get("type", type);
    json.Get("Data", dataJson);
    json.Get("ID", id);
    if(Authentication == true)
    {
        if(strcmp(type.c_str(), "cmd") == 0)
        {
            std::string cmd;
            dataJson.Get("cmd",cmd);
            if(strcmp(cmd.c_str(), "heartbeat") == 0)
                heartbeat(id);
        }
        else if(strcmp(type.c_str(), "UartData") == 0)
        {

        }
    }
    else
    {
        if(strcmp(type.c_str(), "cmd") == 0)
        {
            std::string cmd;
            dataJson.Get("cmd",cmd);
            if(strcmp(cmd.c_str(), "authentication") == 0)
            {
                if(id == 0)
                {
                    authentication(dataJson);        
                    return;
                }
            }
        }
        this->sensingTcpPool->ColesConnect(bev);
    }
}

void SensingOperating::heartbeat(int id)
{    
    neb::CJsonObject json_out;
    neb::CJsonObject json_data;
    this->operation_timer_lock.lock();
    gettimeofday(&this->operation_timer, NULL);
    this->operation_timer_lock.unlock();
    json_data.Add("cmd","heartbeat");
    json_out.Add("res_req","res");
    json_out.Add("type","cmd");
    json_out.Add("Data", json_data);
    this->sendCmd(json_out, id);

    //this->sensingTcpPool->WriteData(this->bev, out.c_str(), out.size());
}
void SensingOperating::authentication(neb::CJsonObject &json)
{
    std::string IdCode;
    std::string Key;
    std::string RealKey;
    json.Get("IdCode", IdCode);
    json.Get("key", Key);
    RealKey =  DataBase::getInstance().CheckUpSensor(IdCode);
    if(strcmp(Key.c_str(), RealKey.c_str()) == 0)
    {
        std::string out;
        this->Authentication = true;
        this->httpApiServer->userList.SensingMapAdd(IdCode, this);
        neb::CJsonObject json_out;
        neb::CJsonObject json_data;
        json_data.Add("cmd","authentication");
        json_data.Add("stat", 0);
        json_out.Add("res_req","res");
        json_out.Add("type","cmd");
        json_out.Add("Data",json_data);
        out = json_out.ToString() + "\r\n";
        this->sendCmd(json_out, 0);
        //this->sensingTcpPool->WriteData(this->bev, out.c_str(), out.size());
    }
    else
    {
            this->sensingTcpPool->ColesConnect(bev);
    }
}
void HttpApiServer_ws_read_cb(struct httpChilent_t * httpChilent)
{
    printf("Http read\r\n");
    HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(httpChilent->httpServer);
    char *req;
    WebSocketOperating * webSocketOperating = httpApiServer->webSocketOperatingList->find(httpChilent);
    if( webSocketOperating != NULL )
    {
        httpApiServer->userList.LatelyOperatorTimeList_lock.lock();
        auto i = httpApiServer->userList.LatelyOperatorTimeList.find(webSocketOperating->User);
        if(i != httpApiServer->userList.LatelyOperatorTimeList.end())
        {
            timeval * tv = &i->second;
            gettimeofday(tv,NULL);
        }
        httpApiServer->userList.LatelyOperatorTimeList_lock.unlock();
    }
    while((req = evbuffer_readln(httpChilent->receive_evbuff, NULL, EVBUFFER_EOL_CRLF)) != NULL)
    {
        neb::CJsonObject json(req);
        httpApiServer->webSocketOperatingList->RecCmd(httpChilent, json);    
        free(req);
        req = NULL;
    }
}
void HttpApiServer_ws_write_cb(struct httpChilent_t * httpChilent)
{
    printf("Http write\r\n");
    HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(httpChilent->httpServer);
    httpApiServer->webSocketOperatingList->write_finish_cb(httpChilent);
    WebSocketOperating * webSocketOperating = httpApiServer->webSocketOperatingList->find(httpChilent);
    if( webSocketOperating != NULL )
    {
        auto i = httpApiServer->userList.LatelyOperatorTimeList.find(webSocketOperating->User);
        if(i != httpApiServer->userList.LatelyOperatorTimeList.end())
        {
            timeval * tv = &i->second;
            gettimeofday(tv,NULL);
        }
    }

}
void HttpApiServer_ws_connect_cb(struct httpChilent_t * httpChilent)
{
    printf("Http connect\r\n");
    HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(httpChilent->httpServer);
    httpApiServer->webSocketOperatingList->Add(httpChilent);
    
}
void HttpApiServer_ws_disconnect_cb(struct httpChilent_t * httpChilent)
{
    HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(httpChilent->httpServer);
    WebSocketOperating * webSocketOperating = httpApiServer->webSocketOperatingList->find(httpChilent);
    if(webSocketOperating != NULL)
    {
        httpApiServer->userList.DelSensing_User_Map_noClose(webSocketOperating->User);
    }
    httpApiServer->webSocketOperatingList->Del(httpChilent);
    printf("Http disconnect\r\n");
}

WebSocketOperating * WebSocketOperatingList::find(struct httpChilent_t * httpChilent)
{
    lock();
    std::map<httpChilent_t *,WebSocketOperating*>::iterator i = this->WebSocketOperatingMap_client.find(httpChilent);
    if(i != this->WebSocketOperatingMap_client.end())
    {
        WebSocketOperating * webSocketOperating = NULL;
        webSocketOperating = i->second;
        unlock();
        return webSocketOperating;
    }
    unlock();
    return NULL;
}
WebSocketOperatingList::WebSocketOperatingList(HttpApiServer *httpApiServer, httpServer_t * httpServer, event_base * main_base)
{
    this->httpServer = httpServer;
    this->httpApiServer = httpApiServer;
    this->main_base = main_base;
}
void WebSocketOperatingList::Del(httpChilent_t * httpChilent)
{
    lock();
    auto i = this->WebSocketOperatingMap_client.find(httpChilent);
    if(i != this->WebSocketOperatingMap_client.end())
    {
        delete i->second;
        this->WebSocketOperatingMap_client.erase(i);
    }
    unlock();
}
void WebSocketOperatingList::Add(httpChilent_t *httpChilent)
{
    WebSocketOperating *webSocketOperating = new WebSocketOperating(this->httpApiServer,this, this->httpServer, httpChilent, this->main_base);
    lock();
    this->WebSocketOperatingMap_client.insert(std::pair<httpChilent_t *,WebSocketOperating *>(httpChilent, webSocketOperating));
    unlock();
}
void WebSocketOperatingList::RecCmd(httpChilent_t * httpChilent, neb::CJsonObject &json)
{
    lock();
    std::map<httpChilent_t *,WebSocketOperating*>::iterator i = this->WebSocketOperatingMap_client.find(httpChilent);
    if(i != this->WebSocketOperatingMap_client.end())
    {
        unlock();
        i->second->RecCmd(json);
        return;
    }
    unlock();
}
bool WebSocketOperatingList::sendCmd(httpChilent_t * httpChilent , neb::CJsonObject &json, int id)
{
    lock();
    std::map<httpChilent_t *,WebSocketOperating*>::iterator i = this->WebSocketOperatingMap_client.find(httpChilent);
    if(i != this->WebSocketOperatingMap_client.end())
    {
        unlock();
        return i->second->sendCmd(json, id);
    }
    unlock();

    return false;
}
void WebSocketOperatingList::write_finish_cb(httpChilent_t * httpChilent)
{
    lock();
    std::map<httpChilent_t *,WebSocketOperating*>::iterator i = this->WebSocketOperatingMap_client.find(httpChilent);
    if(i != this->WebSocketOperatingMap_client.end())
    {
        unlock();
        i->second->write_finish_cb();
        return;
    }
    unlock();
}
void WebSocketOperating_time_cb(evutil_socket_t fd, short events, void *arg)
{
    fd = (evutil_socket_t)fd;
    events = (short)events;
    arg = (void *)arg;
    //WebSocketOperating * webSocketOperating = (WebSocketOperating *)arg;

}
WebSocketOperating::WebSocketOperating(HttpApiServer * httpApiServer , WebSocketOperatingList *webSocketOperatingList,httpServer_t * httpServer, httpChilent_t * httpChilent, event_base * main_base)
{
    this->httpServer = httpServer;
    this->webSocketOperatingList = webSocketOperatingList;
    this->htppChilent = httpChilent;
    this->httpApiServer = httpApiServer;
    this->Authentication = false;
    this->cmd_id = 0;
    this->main_base = main_base;
    gettimeofday(&this->operation_timer, NULL);
        /*检查定时器 */
    event_assign(&ev_time, main_base, -1, EV_PERSIST, WebSocketOperating_time_cb, this);
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = APICLIENT_WS_CHECH_TIME;
    tv.tv_usec = 0; 
    event_add(&ev_time, &tv);
}
WebSocketOperating::~WebSocketOperating()
{
    event_del(&this->ev_time);
}
int WebSocketOperating::idQueue_pop()
{
    int id  = -1;
    this->idQueue_lock.lock();
    if(idQueue.size() > 0)
    {
        id = idQueue.front();
        idQueue.pop();
    }
    this->idQueue_lock.unlock();
    return id;
}
void WebSocketOperating::idQueue_push(int id)
{
    this->idQueue_lock.lock();
    idQueue.push(id); 
    this->idQueue_lock.unlock();
}
void WebSocketOperating::write_finish_cb()
{
    this->sendQueue_lock.lock();
    if(!this->sendQueue.empty())
    {
        neb::CJsonObject json = this->sendQueue.front(); 
        this->sendQueue.pop();
        this->sendQueue_lock.unlock();
        std::string out = json.ToString() + "\r\n";
        HttpServer_webSocket_send(this->htppChilent,out.c_str(),out.size());
        return;
    }
    this->sendQueue_lock.unlock();
}
bool WebSocketOperating::sendCmd(neb::CJsonObject &json,int id)
{
    if(json.IsEmpty())
        return false;
    if(json["ResReq"].IsEmpty()  || json["Data"].IsEmpty() || json["type"].IsEmpty())
        return false;
    std::string res_req;
    std::string type;
    json.Get("ResReq", res_req);
    json.Get("type", type);
    int ID = 0;
    if(id < 0)
    {
        if(strcmp(res_req.c_str(), "Req") != 0)
        {
            return false;
        }
        
        if(json.Get("ID", ID) == false)
        {
            json.Add("ID", this->cmd_id);
        }
        else
        {
            json.Replace("ID", this->cmd_id);
        }
        this->cmd_id++;
    }
    else
    {
        if(strcmp(res_req.c_str(), "Res") != 0)
        {
            return false;
        }
        if(json.Get("ID", ID) == false)
        {
            json.Add("ID", id);
        }
        else
        {
            json.Replace("ID",id);
        }
    }
    
    
    if(strcmp(type.c_str() , "Authentication") != 0 && strcmp(type.c_str(), "DevSetDat") != 0&& strcmp(type.c_str(), "StatDat") != 0)
    {
        return false;
    }
    if(strcmp(res_req.c_str(), "Req") == 0)
    {
        this->sentQueue_lock.lock();
        this->sentQueue.push(json);
        this->sentQueue_lock.unlock();    
    }
    this->sendQueue_lock.lock();
    size_t size = this->sendQueue.size();
    this->sendQueue.push(json);
    this->sendQueue_lock.unlock();

    if(size == 0)
    {
        this->write_finish_cb();
    }
    return true;
}
void *WebSocketOperating_Execute(void * arg)
{
    WebSocketOperating::Thread_arg_t *thread_arg = (WebSocketOperating::Thread_arg_t *)arg;
    //执行 指令
    thread_arg->webSocketOperating->ComExe(*thread_arg->json);
    delete thread_arg->json;
    free(arg);
    return NULL;
}
void *WebSocketOperating_Fruit(void * arg)
{
    WebSocketOperating::Thread_arg_t *thread_arg = (WebSocketOperating::Thread_arg_t *)arg;
    int rep_cnt = 0;
    neb::CJsonObject sent_json;
s1:   
    if(thread_arg->webSocketOperating->sentQueue.size() > 0)
    {
        sent_json = thread_arg->webSocketOperating->sentQueue.front();
        thread_arg->webSocketOperating->sentQueue.pop();
    }
    else
    {
        thread_arg->webSocketOperating->sentQueue_lock.unlock();
        delete thread_arg->json;
        if(thread_arg->webSocketOperating->Authentication == false)
        HttpServer_webSocket_close(thread_arg->webSocketOperating->htppChilent);
        free(arg);
        return NULL;
    }
    int ID_sent = -1;
    int ID_rec = -1;
    sent_json.Get("ID", ID_sent);
    thread_arg->json->Get("ID", ID_rec);
    if(ID_sent != -1 && ID_rec != -1)
    {
        if(ID_rec > ID_sent)
        {
            rep_cnt ++;
            if(rep_cnt > APICLIENT_CMD_LOSS_MAX_NUM)//重试了20次 还没有对应 断开连接
            {
                httpChilent_t * httpChilent = thread_arg->webSocketOperating->htppChilent;
                HttpServer_webSocket_close(httpChilent);
                delete thread_arg->json;
                free(arg);
                return NULL;
            }
            //命令回复 无回应
            thread_arg->webSocketOperating->ComReplyNo(sent_json);
            goto s1;
        }
        else if(ID_rec < ID_sent)//该结果已经失效
        {
            delete thread_arg->json;
            free(arg);
            return NULL;
        }
        else
        {
            //命令回复 处理
            thread_arg->webSocketOperating->ComReply(sent_json,*thread_arg->json);
        }
    }

    delete thread_arg->json;
    free(arg);
    return NULL;
}
void *WebSocketOperating_Separator(void * arg)
{
    WebSocketOperating * webSocketOperating = (WebSocketOperating *)arg;
    size_t size;
    webSocketOperating->recQueue_lock.lock();
    size = webSocketOperating->recQueue.size();
    webSocketOperating->recQueue_lock.unlock();

    while(size != 0)
    {
        std::string res_req;
        WebSocketOperating::Thread_arg_t *cb_arg = (WebSocketOperating::Thread_arg_t *) malloc(sizeof(WebSocketOperating::Thread_arg_t));
        webSocketOperating->recQueue_lock.lock();
        webSocketOperating->recQueue.front().Get("ResReq", res_req);
        cb_arg->json = new neb::CJsonObject(webSocketOperating->recQueue.front());
        webSocketOperating->recQueue.pop();
        webSocketOperating->recQueue_lock.unlock();
        cb_arg->webSocketOperating = webSocketOperating;
        if(strcmp(res_req.c_str(), "Res") == 0)
        {
            WebSocketOperating_Fruit(cb_arg);
        }
        else
        {
            WebSocketOperating_Execute(cb_arg);
        }
        webSocketOperating->recQueue_lock.lock();
        size = webSocketOperating->recQueue.size();
        webSocketOperating->recQueue_lock.unlock();
    }

    return NULL;
}
void WebSocketOperating::RecCmd(neb::CJsonObject &json)
{
    if(json.IsEmpty())
    {
        HttpServer_webSocket_close(this->htppChilent);
        return;
    }
    if(json["ResReq"].IsEmpty() || json["ID"].IsEmpty() || json["Data"].IsEmpty() || json["type"].IsEmpty())
    {
        HttpServer_webSocket_close(this->htppChilent);
        return;
    }
    threadpool_t * threadpool = HttpServer_GetThreadPool(this->httpServer);
    std::string res_req;
    std::string type;
    json.Get("ResReq", res_req);
    json.Get("type", type);
    int id = 0;
    if(json.Get("ID", id) == false)
    {
        HttpServer_webSocket_close(this->htppChilent);
        return;
    }
    if(strcmp(res_req.c_str() , "Res") != 0 && strcmp(res_req.c_str(), "Req") != 0)
    {
        HttpServer_webSocket_close(this->htppChilent);
        return;
    }
    if(strcmp(type.c_str() , "Authentication") != 0 && strcmp(type.c_str(), "DevSetDat") != 0&& strcmp(type.c_str(), "StatDat") != 0)
    {
        HttpServer_webSocket_close(this->htppChilent);
        return;
    }
    recQueue_lock.lock();
    size_t recQueue_size = recQueue.size();
    recQueue.push(json);
    if(recQueue_size == 0)
    {
        threadpool_add(threadpool, WebSocketOperating_Separator, this);
    }
    recQueue_lock.unlock();
}


void HttpApiServer_sensing_read_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg)
{
    printf("%s:%d rec!\r\n",ipPort.ip, ipPort.port);
    ipPort = (TcpConnectPool::IpPort_t &)ipPort;
    SensingOperatingList * sensingOperatingList = ((HttpApiServer *)arg)->sensingOperatingList;
    tcpConnectPool = (TcpConnectPool *)tcpConnectPool;
    bufferevent_lock(bev);
    evbuffer *evb = bufferevent_get_input(bev);
    char *req;
    while((req = evbuffer_readln(evb, NULL, EVBUFFER_EOL_CRLF)) != NULL)
    {
        std::string str(req);
        neb::CJsonObject json(str);
        sensingOperatingList->RecCmd(bev, json);    
        free(req);
        req = NULL;
    }
    bufferevent_unlock(bev);
}
void HttpApiServer_sensing_write_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg)
{
    printf("%s:%d weite fulfil!\r\n",ipPort.ip, ipPort.port);
    ipPort = (TcpConnectPool::IpPort_t &)ipPort;
    tcpConnectPool = (TcpConnectPool *)tcpConnectPool;
    SensingOperatingList * sensingOperatingList = ((HttpApiServer *)arg)->sensingOperatingList;
    sensingOperatingList->write_finish_cb(bev);
}
void HttpApiServer_sensing_connect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg)
{
    printf("%s:%d connect!\r\n",ipPort.ip, ipPort.port);
    tcpConnectPool = (TcpConnectPool *)tcpConnectPool;
    SensingOperatingList * sensingOperatingList = ((HttpApiServer *)arg)->sensingOperatingList;
    sensingOperatingList->Add(bev, ipPort);
}
void HttpApiServer_sensing_disconnect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev,TcpConnectPool * tcpConnectPool, void * arg)
{
    printf("%s:%d disconnect!\r\n",ipPort.ip, ipPort.port);
    ipPort = (TcpConnectPool::IpPort_t &)ipPort;
    tcpConnectPool = (TcpConnectPool *)tcpConnectPool;
    SensingOperatingList * sensingOperatingList = ((HttpApiServer *)arg)->sensingOperatingList;
    SensingOperating * sensingOperating = sensingOperatingList->find(bev);
    if(sensingOperating != NULL)
    {
        sensingOperatingList->httpApiServer->userList.SensingMapDel(sensingOperating);
    }
    sensingOperatingList->Del(bev);
}
void HttpApiServer_sensing_error_cb(TcpConnectPool * tcpConnectPool, TcpConnectPool::ErrorType errorType, std::string &err_str, void * arg)
{
    tcpConnectPool = (TcpConnectPool *)tcpConnectPool;
    SensingOperatingList * sensingOperatingList = ((HttpApiServer *)arg)->sensingOperatingList;
    sensingOperatingList = (SensingOperatingList *)sensingOperatingList;
    De_fprintf(stderr, "%s ] %s" , TcpConnectPool::ErrorTypeToString(errorType).c_str(),err_str.c_str());    
    //传感端 TCP 连接池 异常 退出
    if(TcpConnectPool::ErrorType::ERROR_ACCEPT == errorType || errorType == TcpConnectPool::ErrorType::ERROR_BASE_BREAK)
    {
        ((HttpApiServer *)arg)->end_loop();//服务器退出
    }
}
SensingOperating * SensingOperatingList::find(struct bufferevent * bev)
{
    lock();
    std::map<bufferevent *,SensingOperating*>::iterator i = this->SensingOperatingMap_bev.find(bev);
    if(i != this->SensingOperatingMap_bev.end())
    {
        SensingOperating * sensingOperating = NULL;
        sensingOperating = i->second;
        unlock();
        return sensingOperating;;
    }
    unlock();
    return NULL;
}

SensingOperatingList::SensingOperatingList(HttpApiServer * httpApiServer, TcpConnectPool * sensingTcpPool, event_base * main_base)
{
    this->sensingTcpPool = sensingTcpPool;
    this->httpApiServer = httpApiServer;
    this->main_base = main_base;
}
void SensingOperatingList::Del(bufferevent * bev)
{
    lock();
    auto i = this->SensingOperatingMap_bev.find(bev);
    if(i != this->SensingOperatingMap_bev.end())
    {
        delete i->second;
        this->SensingOperatingMap_bev.erase(bev);
    }
    unlock();
}
void SensingOperatingList::Add(bufferevent *bev, TcpConnectPool::IpPort_t & ipPort)
{
    SensingOperating *sensingOperating = new SensingOperating(this->httpApiServer ,this, this->sensingTcpPool, bev, ipPort, this->main_base);
    lock();
    this->SensingOperatingMap_bev.insert(std::pair<bufferevent *,SensingOperating *>(bev, sensingOperating));
    unlock();
}
void SensingOperatingList::RecCmd(bufferevent * bev, neb::CJsonObject &json)
{
    std::map<bufferevent *,SensingOperating*>::iterator i = this->SensingOperatingMap_bev.find(bev);
    if(i != this->SensingOperatingMap_bev.end())
    {
        unlock();
        i->second->RecCmd(json);
        return;
    }
    unlock();
}
bool SensingOperatingList::sendCmd(bufferevent * bev, neb::CJsonObject &json, int id)
{
    lock();
    std::map<bufferevent *,SensingOperating*>::iterator i = this->SensingOperatingMap_bev.find(bev);
    if(i != this->SensingOperatingMap_bev.end())
    {
        unlock();
        return i->second->sendCmd(json, id);
    }
    unlock();

    return false;
}
void SensingOperatingList::write_finish_cb(bufferevent * bev)
{
    std::map<bufferevent *,SensingOperating*>::iterator i = this->SensingOperatingMap_bev.find(bev);
    if(i != this->SensingOperatingMap_bev.end())
    {
        unlock();
        i->second->write_finish_cb();
        return;
    }
    unlock();
}
void SensingOperating_time_cb(evutil_socket_t fd, short events, void *arg)
{
    fd = (evutil_socket_t)fd;
    events = (short)events;
    SensingOperating * sensingOperating = (SensingOperating *)arg;
    //心跳超时检测
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sensingOperating->operation_timer_lock.lock();
    if( tv.tv_sec - sensingOperating->operation_timer.tv_sec  >= APICLIENT_HEARBEAT_SENSING_TIME_OUT)
    {
        sensingOperating->sensingTcpPool->ColesConnect(sensingOperating->bev);
    }
    sensingOperating->operation_timer_lock.unlock();
}
SensingOperating::~SensingOperating()
{
    event_del(&ev_time);
}
SensingOperating::SensingOperating(HttpApiServer * httpApiServer , SensingOperatingList *sensingOperatingList,TcpConnectPool *sensingTcpPool, bufferevent *bev, TcpConnectPool::IpPort_t & ipPort, event_base * main_base)
{
    this->sensingTcpPool = sensingTcpPool;
    this->sensingOperatingList = sensingOperatingList;
    this->bev = bev;
    this->httpApiServer = httpApiServer;
    memcpy(&this->ipPort, &ipPort, sizeof(TcpConnectPool::IpPort_t));
    this->Authentication = false;
    cmd_id = 0;
    this->main_base = main_base;
    gettimeofday(&this->operation_timer, NULL);
        /*检查定时器 */
    event_assign(&ev_time, mreq["type"]req["type"]ain_base, -1, EV_PERSIST, SensingOperating_time_cb, this);
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = APICLIENT_SENSING_CHECH_TIME;
    tv.tv_usec = 0; 
    event_add(&ev_time, &tv);
}
int SensingOperating::idQueue_pop()
{
    int id  = -1;
    this->idQueue_lock.lock();
    if(idQueue.size() > 0)
    {
        id = idQueue.front();
        idQueue.pop();
    }
    this->idQueue_lock.unlock();
    return id;
}
void SensingOperating::idQueue_push(int id)
{
    this->idQueue_lock.lock();
    idQueue.push(id); 
    this->idQueue_lock.unlock();
}
void SensingOperating::write_finish_cb()
{
    this->sendQueue_lock.lock();
    if(!this->sendQueue.empty())
    {
        neb::CJsonObject json = this->sendQueue.front(); 
        this->sendQueue.pop();
        this->sendQueue_lock.unlock();
        std::string out = json.ToString() + "\r\n";
        this->sensingTcpPool->WriteData(bev, out.c_str(), out.size());
        return;
    }
    this->sendQueue_lock.unlock();
}
bool SensingOperating::sendCmd(neb::CJsonObject &json, int id)
{
    if(json.IsEmpty())
        return false;
    if(json["res_req"].IsEmpty()  || json["Data"].IsEmpty() || json["type"].IsEmpty())
        return false;
    std::string res_req;
    std::string type;
    json.Get("res_req", res_req);
    json.Get("type", type);
    int ID = 0;
    if(id < 0)
    {
        if(strcmp(res_req.c_str(), "req") != 0)
        {
            return false;
        }
        
        if(json.Get("ID", ID) == false)
        {
            json.Add("ID", this->cmd_id);
        }
        else
        {
            json.Replace("ID", this->cmd_id);
        }
        this->cmd_id++;
    }
    else
    {
        if(strcmp(res_req.c_str(), "res") != 0)
        {
            return false;
        }
        if(json.Get("ID", ID) == false)
        {
            json.Add("ID", id);
        }
        else
        {
            json.Replace("ID",id);
        }
    }
    if(strcmp(type.c_str() , "cmd") != 0 && strcmp(type.c_str(), "UartData") != 0)
    {
        return false;
    }
    this->sendQueue_lock.lock();
    size_t size = this->sendQueue.size();
    this->sendQueue.push(json);
    this->sendQueue_lock.unlock();
    if(strcmp(res_req.c_str(), "req") == 0)
    {
        this->sentQueue_lock.lock();
        this->sentQueue.push(json);
        this->sentQueue_lock.unlock();    
    }
    if(size == 0)
    {
        this->write_finish_cb();
    }

    return true;
}
void *SensingOperating_Execute(void * arg)
{
    SensingOperating::Thread_arg_t *thread_arg = (SensingOperating::Thread_arg_t *)arg;
    //执行 指令
    thread_arg->sensingOperating->ComExe(*thread_arg->json);
    delete thread_arg->json;
    free(arg);
    return NULL;
}
void *SensingOperating_Fruit(void * arg)
{
    SensingOperating::Thread_arg_t *thread_arg = (SensingOperating::Thread_arg_t *)arg;
    int rep_cnt = 0;
    neb::CJsonObject sent_json;
s1:
    thread_arg->sensingOperating->sentQueue_lock.lock();
    if(thread_arg->sensingOperating->sentQueue.size() > 0)
    {
        sent_json = thread_arg->sensingOperating->sentQueue.front();
        thread_arg->sensingOperating->sentQueue.pop();
    }
    else
    {
        thread_arg->sensingOperating->sentQueue_lock.unlock();
        delete thread_arg->json;
        if(thread_arg->sensingOperating->Authentication == false)
            thread_arg->sensingOperating->sensingTcpPool->ColesConnect(thread_arg->sensingOperating->bev);
        free(arg);
        return NULL;
    }
    thread_arg->sensingOperating->sentQueue_lock.unlock();
    int ID_sent = -1;
    int ID_rec = -1;
    sent_json.Get("ID", ID_sent);
    thread_arg->json->Get("ID", ID_rec);
    if(ID_sent != -1 && ID_rec != -1)
    {
        if(ID_rec > ID_sent)
        {
            rep_cnt ++;
            if(rep_cnt > APICLIENT_CMD_LOSS_MAX_NUM)//重试了20次 还没有对应 断开连接
            {
                bufferevent * bev = thread_arg->sensingOperating->bev;
                thread_arg->sensingOperating->sensingTcpPool->ColesConnect(bev);
                delete thread_arg->json;
                free(arg);
                return NULL;
            }
            //命令回复 无回应
            thread_arg->sensingOperating->ComReplyNo(sent_json);
            goto s1;
        }
        else if(ID_rec < ID_sent)//该结果已经失效
        {
            delete thread_arg->json;
            free(arg);
            return NULL;
        }
        else
        {
            //命令回复 处理
            thread_arg->sensingOperating->ComReply(sent_json,*thread_arg->json);
        }
    }

    delete thread_arg->json;
    free(arg);
    return NULL;
}
void *SensingOperating_Separator(void * arg)
{
    SensingOperating * sensingOperating = (SensingOperating *)arg;
    size_t size;
    sensingOperating->recQueue_lock.lock();
    size = sensingOperating->recQueue.size();
    sensingOperating->recQueue_lock.unlock();

    while(size != 0)
    {
        std::string res_req;
        SensingOperating::Thread_arg_t *cb_arg = (SensingOperating::Thread_arg_t *) malloc(sizeof(SensingOperating::Thread_arg_t));
        sensingOperating->recQueue_lock.lock();
        sensingOperating->recQueue.front().Get("res_req", res_req);
        cb_arg->json = new neb::CJsonObject(sensingOperating->recQueue.front());
        sensingOperating->recQueue.pop();
        sensingOperating->recQueue_lock.unlock();
        cb_arg->sensingOperating = sensingOperating;
        if(strcmp(res_req.c_str(), "res") == 0)
        {
            SensingOperating_Fruit(cb_arg);
        }
        else
        {
            SensingOperating_Execute(cb_arg);
        }
        sensingOperating->recQueue_lock.lock();
        size = sensingOperating->recQueue.size();
        sensingOperating->recQueue_lock.unlock();
    }

    return NULL;
}
void SensingOperating::RecCmd(neb::CJsonObject &json)
{
    if(json.IsEmpty())
    {
        this->sensingTcpPool->ColesConnect(this->bev);
        return;
    }
    if(json["res_req"].IsEmpty() || json["ID"].IsEmpty() || json["Data"].IsEmpty() || json["type"].IsEmpty())
    {
        this->sensingTcpPool->ColesConnect(this->bev);
        return;
    }
    threadpool_t * threadpool = sensingTcpPool->GetThreadpool();
    std::string res_req;
    std::string type;
    json.Get("res_req", res_req);
    json.Get("type", type);
    int id = -1;
    json.Get("ID", id);
    if(json.Get("ID", id) == false)
    {
        this->sensingTcpPool->ColesConnect(this->bev);
        return;
    }
    if(strcmp(res_req.c_str() , "res") != 0 && strcmp(res_req.c_str(), "req") != 0)
    {
        this->sensingTcpPool->ColesConnect(this->bev);
        return;
    }
    if(strcmp(type.c_str() , "cmd") != 0 && strcmp(type.c_str(), "UartData") != 0)
    {
        this->sensingTcpPool->ColesConnect(this->bev);
        return;
    }
    recQueue_lock.lock();
    size_t recQueue_size = recQueue.size();
    recQueue.push(json);
    if(recQueue_size == 0)
    {
        threadpool_add(threadpool, SensingOperating_Separator, this);
    }
    recQueue_lock.unlock();
}


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
    LatelyOperatorTimeList_lock.lock();
    LatelyOperatorTimeList.insert(std::pair<std::string,timeval>(user, tv));
    LatelyOperatorTimeList_lock.unlock();
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
            DelSensing_User_Map(i->second);
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
            DelSensing_User_Map(user);
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
        LatelyOperatorTimeList_lock.lock();
        gettimeofday(&LatelyOperatorTimeList.find(user)->second,NULL);
        LatelyOperatorTimeList_lock.unlock();
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
        LatelyOperatorTimeList_lock.lock();
        gettimeofday(&LatelyOperatorTimeList.find(user)->second,NULL);
        LatelyOperatorTimeList_lock.unlock();
        std::string key = i->second;
        unlock();
        return key;
    }
    unlock();
    return "";
}

void HttpApiServer::UserListClass::AddSensing_User_Map(std::string user, SensingOperating * sensingOperating)
{
    if(!this->findKey(user).empty())
    {
        User_Sensing_Map_lock.lock();
        auto i = User_Sensing_Map_user.find(user);
        if(i!= User_Sensing_Map_user.end())
        {
            auto j = User_Sensing_Map_client.find(user);
            if(j != User_Sensing_Map_client.end())
            {
                HttpServer_webSocket_close_on_cb(j->second);
                HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(j->second->httpServer);
                httpApiServer->webSocketOperatingList->Del(j->second);
                User_Sensing_Map_client.erase(user);
            }
            User_Sensing_Map_sensing.erase(i->second);
            User_Sensing_Map_user.erase(i);
        }
        User_Sensing_Map_user.insert(std::pair<std::string , SensingOperating *>(user, sensingOperating));
        User_Sensing_Map_sensing.insert(std::pair<SensingOperating *, std::string>( sensingOperating ,user));
        User_Sensing_Map_lock.unlock();
    }
}
SensingOperating * HttpApiServer::UserListClass::User_Sensing_MapfindSensing(std::string user)
{
    User_Sensing_Map_lock.lock();
    std::map<std::string , SensingOperating *>::iterator i = User_Sensing_Map_user.find(user);
    if(i != User_Sensing_Map_user.end() )
    {
        SensingOperating * sensingOperating = NULL; 
        sensingOperating = i->second;
        User_Sensing_Map_lock.unlock();
        return sensingOperating;
    }
    User_Sensing_Map_lock.unlock();
    return NULL;
}
std::string HttpApiServer::UserListClass::User_Sensing_MapfindUser(SensingOperating *sensingOperating)
{
    std::string user; 
    User_Sensing_Map_lock.lock();
    std::map< SensingOperating * ,std::string >::iterator i = User_Sensing_Map_sensing.find(sensingOperating);
    if(i != User_Sensing_Map_sensing.end() )
    {
        user = i->second;
        User_Sensing_Map_lock.unlock();
        return user;
    }
    User_Sensing_Map_lock.unlock();
    return user;
}

httpChilent_t * HttpApiServer::UserListClass::User_Sensing_MapfindWebSocket(std::string user)
{
    this->User_Sensing_Map_lock.lock();
    auto i = User_Sensing_Map_client.find(user); 
    if(i != User_Sensing_Map_client.end())
    {
        httpChilent_t * httpChilent = NULL;
        httpChilent = i->second;
        this->User_Sensing_Map_lock.lock();
        return httpChilent;
    }
    this->User_Sensing_Map_lock.lock();
    return NULL;
}
void HttpApiServer::UserListClass::AddSensing_User_Map_client(std::string user,httpChilent_t *httpChilent)
{
    this->User_Sensing_Map_lock.lock();
    this->User_Sensing_Map_client.insert(std::pair<std::string, httpChilent_t *>(user, httpChilent));
    this->User_Sensing_Map_lock.unlock();
}
void HttpApiServer::UserListClass::DelSensing_User_Map(std::string user)
{
    User_Sensing_Map_lock.lock();
    std::map<std::string , SensingOperating *>::iterator i = User_Sensing_Map_user.find(user);
    if(i != User_Sensing_Map_user.end())
    {
        auto j = User_Sensing_Map_client.find(user);
        if(j != User_Sensing_Map_client.end())
        {
            HttpServer_webSocket_close_on_cb(j->second);
            HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(j->second->httpServer);
            httpApiServer->webSocketOperatingList->Del(j->second);
            User_Sensing_Map_client.erase(j);
        }
        User_Sensing_Map_sensing.erase(i->second);
        User_Sensing_Map_user.erase(i);
    }
    User_Sensing_Map_lock.unlock();
}
void HttpApiServer::UserListClass::DelSensing_User_Map(SensingOperating * sensingOperating)
{
    User_Sensing_Map_lock.lock();
    std::map< SensingOperating *,std::string >::iterator i = User_Sensing_Map_sensing.find(sensingOperating);
    if(i != User_Sensing_Map_sensing.end())
    {
        auto j = User_Sensing_Map_client.find(i->second);
        if(j != User_Sensing_Map_client.end())
        {
            HttpServer_webSocket_close_on_cb(j->second);

            HttpApiServer *httpApiServer = (HttpApiServer *) httpServer_getArg(j->second->httpServer);
            httpApiServer->webSocketOperatingList->Del(j->second);
            User_Sensing_Map_client.erase(j);
        }
        User_Sensing_Map_user.erase(i->second);
        User_Sensing_Map_sensing.erase(i);
    }
    User_Sensing_Map_lock.unlock();
}
void HttpApiServer::UserListClass::DelSensing_User_Map_noClose(std::string user)
{
    User_Sensing_Map_lock.lock();
    std::map<std::string , SensingOperating *>::iterator i = User_Sensing_Map_user.find(user);
    if(i != User_Sensing_Map_user.end())
    {
        auto j = User_Sensing_Map_client.find(user);
        if(j != User_Sensing_Map_client.end())
        {
            User_Sensing_Map_client.erase(j);
        }
        User_Sensing_Map_client.erase(user);
        User_Sensing_Map_sensing.erase(i->second);
        User_Sensing_Map_user.erase(i);
    }
    User_Sensing_Map_lock.unlock();

}

void HttpApiServer::UserListClass::SensingMapAdd(std::string id, SensingOperating * sensingOperating)
{
    Sensing_Map_lock.lock();
    SensingMap_id.insert(std::pair<std::string , SensingOperating *>(id, sensingOperating));
    SensingMap_sensing.insert(std::pair< SensingOperating *, std::string>(sensingOperating, id));
    Sensing_Map_lock.unlock();
}
void HttpApiServer::UserListClass::SensingMapDel(SensingOperating * sensingOperating)
{
    Sensing_Map_lock.lock();
    std::map< SensingOperating *,std::string >::iterator i = SensingMap_sensing.find(sensingOperating);
    if(i != SensingMap_sensing.end())
    {
        DelSensing_User_Map(i->first);
        SensingMap_id.erase(i->second);
        SensingMap_sensing.erase(i);
    }
    Sensing_Map_lock.unlock();
    
}
void HttpApiServer::UserListClass::SensingMapDel(std::string id)
{
    Sensing_Map_lock.lock();
    std::map<std::string, SensingOperating *>::iterator i = SensingMap_id.find(id);
    if(i != SensingMap_id.end())
    {
        DelSensing_User_Map(i->second);
        SensingMap_sensing.erase(i->second);
        SensingMap_id.erase(i);
    }
    Sensing_Map_lock.unlock();
}
SensingOperating * HttpApiServer::UserListClass::SensingMapfind(std::string id)
{
    Sensing_Map_lock.lock();
    std::map<std::string, SensingOperating *>::iterator i = SensingMap_id.find(id);
    if(i != SensingMap_id.end())
    {
        SensingOperating *sensingOperating =NULL;
        sensingOperating = i->second;
        Sensing_Map_lock.unlock();
        return sensingOperating;
    }
    Sensing_Map_lock.unlock();
    return NULL;
}
std::string HttpApiServer::UserListClass::SensingMapfind(SensingOperating * sensingOperating)
{
    std::string id;
    Sensing_Map_lock.lock();
    std::map< SensingOperating *, std::string>::iterator i = SensingMap_sensing.find(sensingOperating);
    if(i != SensingMap_sensing.end())
    {
        id = i->second;
        Sensing_Map_lock.unlock();
        return id;
    }
    Sensing_Map_lock.unlock();
    return id;
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
                    {
                        SensingOperating * sensingOperating = NULL;
                        if( (sensingOperating =  this->userList.SensingMapfind(IdCode)) != NULL)
                        {
                            this->userList.AddSensing_User_Map(user, sensingOperating);
                            rec = 0;
                        }
                        else
                            rec = -3;
                    }
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
    httpApiServer->userList.LatelyOperatorTimeList_lock.lock();
    httpApiServer->userList.lock();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    for(auto i = httpApiServer->userList.LatelyOperatorTimeList.begin();
        i != httpApiServer->userList.LatelyOperatorTimeList.end();)
    {
        if(tv.tv_sec -  i->second.tv_sec >= APICLIENT_TIMEOUT )
        {
            auto j =  httpApiServer->userList.userList.find(i->first);
            httpApiServer->userList.DelSensing_User_Map(j->first);
            auto x =  httpApiServer->userList.keyList.find(i->first);
            httpApiServer->userList.keyList.erase(x);
            httpApiServer->userList.userList.erase(j);
            httpApiServer->userList.LatelyOperatorTimeList.erase(i++);
        }
        else
            i++;
    }
    httpApiServer->userList.unlock();
    httpApiServer->userList.LatelyOperatorTimeList_lock.unlock();

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
        httpServer_setWebSocket_read_cb(httpServer, HttpApiServer_ws_read_cb, "/connect-sensing");
        httpServer_setWebSocket_cb(httpServer, HttpApiServer_ws_write_cb, HttpApiServer_ws_connect_cb, HttpApiServer_ws_disconnect_cb); 
        try
        {
            this->sensingTcpPool = new TcpConnectPool(this->main_base, 6666, this, HttpApiServer_sensing_read_cb, HttpApiServer_sensing_connect_cb,  HttpApiServer_sensing_disconnect_cb);
            this->sensingTcpPool->Set_Wirter_cb(HttpApiServer_sensing_write_cb);
            this->sensingTcpPool->Set_Error_cb(HttpApiServer_sensing_error_cb);
            this->sensingOperatingList = new SensingOperatingList(this, this->sensingTcpPool, main_base);
            this->webSocketOperatingList = new WebSocketOperatingList(this, this->httpServer, main_base);
        }
        catch(std::string err_str) 
        {
            httpServer_free(this->httpServer);
            event_base_free(this->listen_base);
            event_base_free(this->main_base);
            error.IsException = true;
            error.errorType = HttpApiServerException::ErrorType::COMMONLY_ERROR;
            error.err_str = "new TcpConnectPool fail:" +err_str;
            break;
        }

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int rec = pthread_create(&this->listen_th, &attr,HttpApiServer_httplisten_thread,(void *)this);
        pthread_attr_destroy(&attr);
        if(rec != 0)
        {
            httpServer_free(this->httpServer);
            event_base_free(this->listen_base);
            event_base_free(this->main_base);
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
    if(sensingTcpPool != NULL)
    {
        delete sensingTcpPool;
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



