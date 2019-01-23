/*************************************************************************
# > File Name: /home/sszl/Src/IotHttpWebsocketServer/HttpApiServer.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-09 18:04:59
# > Revise Time: 2018-11-09 15:12:55
 ************************************************************************/

#ifndef _HTTPAPISERVER_H
#define _HTTPAPISERVER_H
#include "Http_websocket/include/evbase_threadpool.h" 
#include "Http_websocket/include/ev_httpd.h"
#include "database.h"
#include <signal.h>
#include <map>
#include <queue>
#include "CJsonObject/CJsonObject.hpp"
#include "TcpConnectPool.h"
#include "Http_websocket/include/Threadpool.h"
class HttpApiServer;
class HttpApiServerException
{
    friend class HttpApiServer;
    friend void ev_time_inspect_cb(evutil_socket_t fd, short events, void *arg);
public:
    enum ErrorType
    {
        COMMONLY_ERROR,
        DEADLY_ERROE
    };
    ErrorType errorType;
    HttpApiServerException();
    std::string toString();
private:
    std::string err_str;
    bool IsException;
};
class WebSocketOperatingList;
class WebSocketOperating 
{
    friend class HttpApiServer;
    friend class SensingOperating; 
    friend class SensingOperatingList; 
    friend class WebSocketOperatingList;
    friend void *WebSocketOperating_Separator(void * arg);
    friend void *WebSocketOperating_Execute(void * arg);
    friend void *WebSocketOperating_Fruit(void * arg);
    friend void HttpApiServer_ws_disconnect_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_read_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_write_cb(struct httpChilent_t * httpChilent);
private:
    typedef struct 
    {
        WebSocketOperating * webSocketOperating;
        neb::CJsonObject * json;
    }Thread_arg_t;
    CMutex sendQueue_lock;
    std::queue<neb::CJsonObject> sendQueue;//发送队列
    CMutex recQueue_lock;
    std::queue<neb::CJsonObject> recQueue;//接收队列
    CMutex sentQueue_lock;
    std::queue<neb::CJsonObject> sentQueue;//以发送队列

    
    CMutex idQueue_lock;
    std::queue<int> idQueue;
    void idQueue_push(int id); 
    int idQueue_pop(); 


    CMutex operation_timer_lock;
    struct timeval operation_timer;
    struct event ev_time;
    struct event_base *main_base;

    HttpApiServer * httpApiServer;
    httpServer_t *httpServer;
    httpChilent_t * htppChilent;
    WebSocketOperatingList * webSocketOperatingList;
    bool Authentication;
    std::string User;

    int cmd_id;
    ~WebSocketOperating();
    WebSocketOperating(HttpApiServer * httpApiServer, WebSocketOperatingList * sensingOperatingList, httpServer_t * httpServer, httpChilent_t * httpChilent, event_base * main_base);
    void RecCmd(neb::CJsonObject &json);
    bool sendCmd(neb::CJsonObject &json, int id = -1);
    void ComReply(neb::CJsonObject &req,neb::CJsonObject &res);
    void ComReplyNo(neb::CJsonObject &req);
    void ComExe(neb::CJsonObject &json);

    void write_finish_cb();

    void authentication(std::string key);
    void CarTest(int id);
};
class WebSocketOperatingList : private CMutex
{
    friend class HttpApiServer;
    friend class SensingOperatingList;
    friend class SensingOperating;
    friend void HttpApiServer_ws_read_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_write_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_connect_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_disconnect_cb(struct httpChilent_t * httpChilent);
private:
    std::map<httpChilent_t *, WebSocketOperating *> WebSocketOperatingMap_client;
    void Add(httpChilent_t * httpChilent);
    void Del(httpChilent_t * httpChilent);
    void RecCmd(httpChilent_t * httpChilent, neb::CJsonObject &json);
    bool sendCmd(httpChilent_t * httpChilent, neb::CJsonObject &json , int id = -1);
    void write_finish_cb(httpChilent_t * httpChilent);
    httpServer_t *httpServer;
    HttpApiServer * httpApiServer;
    struct event_base *main_base;
    WebSocketOperatingList(HttpApiServer * httpApiServer ,httpServer_t *httpServer, event_base * main_base);
    WebSocketOperating * find(struct httpChilent_t * httpChilent);
};

class SensingOperatingList;
class SensingOperating 
{
    friend class HttpApiServer;
    friend class WebSocketOperating;
    friend class SensingOperatingList;
    friend void *SensingOperating_Separator(void * arg);
    friend void *SensingOperating_Execute(void * arg);
    friend void *SensingOperating_Fruit(void * arg);
    friend void HttpApiServer_sensing_disconnect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev,TcpConnectPool * tcpConnectPool, void * arg);
    friend void SensingOperating_time_cb(evutil_socket_t fd, short events, void *arg);
private:
    typedef struct 
    {
        SensingOperating * sensingOperating;
        neb::CJsonObject * json;
    }Thread_arg_t;
    CMutex sendQueue_lock;
    std::queue<neb::CJsonObject> sendQueue;//发送队列
    CMutex recQueue_lock;
    std::queue<neb::CJsonObject> recQueue;//接收队列
    CMutex sentQueue_lock;
    std::queue<neb::CJsonObject> sentQueue;//以发送队列

    CMutex idQueue_lock;
    std::queue<int> idQueue;
    void idQueue_push(int id); 
    int idQueue_pop(); 

    TcpConnectPool *sensingTcpPool;
    bufferevent * bev;
    HttpApiServer * httpApiServer;
    TcpConnectPool::IpPort_t  ipPort;
    SensingOperatingList * sensingOperatingList;
    bool Authentication;
    std::string IdCode;
    
    int cmd_id;
    CMutex operation_timer_lock;
    struct timeval operation_timer;
    struct event ev_time;
    struct event_base *main_base;
    SensingOperating(HttpApiServer * httpApiServer , SensingOperatingList * sensingOperatingList,TcpConnectPool * tcpConnectPool, bufferevent *bev, TcpConnectPool::IpPort_t & ipPort, event_base * main_base);
    ~SensingOperating();
    void RecCmd(neb::CJsonObject &json);
    bool sendCmd(neb::CJsonObject &json, int id = -1);// id = -1 发送命令 id >= 0 发送回应
    void write_finish_cb();
    void ComReply(neb::CJsonObject &req,neb::CJsonObject &res);
    void ComReplyNo(neb::CJsonObject &req);
    void ComExe(neb::CJsonObject &json);

    void authentication(neb::CJsonObject &json);

    void heartbeat(int id);

};
class SensingOperatingList : private CMutex
{
    friend class HttpApiServer;
    friend class SensingOperating;
    friend void *SensingOperating_Execute(void * arg);
    friend void *SensingOperating_Fruit(void * arg);
    friend void HttpApiServer_sensing_read_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_write_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_connect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_disconnect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev,TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_error_cb(TcpConnectPool * tcpConnectPool, TcpConnectPool::ErrorType errorType, std::string &err_str, void * arg);
private:
    std::map<bufferevent *, SensingOperating *> SensingOperatingMap_bev;
    void Add(bufferevent *bev, TcpConnectPool::IpPort_t & ipPort);
    void Del(bufferevent * bev);
    void RecCmd(bufferevent * bev, neb::CJsonObject &json);
    bool sendCmd(bufferevent * bev, neb::CJsonObject &json , int id = -1);
    void write_finish_cb(bufferevent * bev);
    
    struct event_base *main_base;
    TcpConnectPool *sensingTcpPool;
    HttpApiServer * httpApiServer;
    SensingOperatingList(  HttpApiServer  * httpApiServer, TcpConnectPool * sensingTcpPool , event_base * main_base);
    SensingOperating * find(struct bufferevent  * bev);
};
class HttpApiServer : private CMutex
{
    friend class SensingOperating;
    friend class WebSocketOperating;
    friend void *HttpApiServer_httplisten_thread(void *arg);
    friend void ev_time_inspect_cb(evutil_socket_t fd, short events, void *arg);
    friend void httpHandler_cb(struct httpChilent_t * httpChilent);
    friend void ws_read_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_sensing_read_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_write_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_connect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev, TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_disconnect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev,TcpConnectPool * tcpConnectPool, void * arg);
    friend void HttpApiServer_sensing_error_cb(TcpConnectPool * tcpConnectPool, TcpConnectPool::ErrorType errorType, std::string &err_str, void * arg);
    friend void HttpApiServer_ws_read_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_write_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_connect_cb(struct httpChilent_t * httpChilent);
    friend void HttpApiServer_ws_disconnect_cb(struct httpChilent_t * httpChilent);
public:
    HttpApiServer(u_short port);
    ~HttpApiServer();
    void start_loop();
    void end_loop();
public:
    /* 异常 */
    HttpApiServerException error;
private:
    SensingOperatingList * sensingOperatingList;
    WebSocketOperatingList * webSocketOperatingList;
    TcpConnectPool *sensingTcpPool;
    httpServer_t *httpServer;
    pthread_t listen_th;
    struct event_base *listen_base;
    struct event_base *main_base;
    u_short http_port;
    /* 定时器 */
    struct event ev_time_inspect;
    /* 在线用户 */
    class UserListClass : private CMutex
    {
        friend class HttpApiServer;
        friend class SensingOperating;
        friend class WebSocketOperating;
        friend void ev_time_inspect_cb(evutil_socket_t fd, short events, void *arg);
        friend void HttpApiServer_ws_read_cb(struct httpChilent_t * httpChilent);
        friend void HttpApiServer_ws_disconnect_cb(struct httpChilent_t * httpChilent);
        friend void HttpApiServer_ws_write_cb(struct httpChilent_t * httpChilent);
        friend void HttpApiServer_sensing_disconnect_cb(TcpConnectPool::IpPort_t & ipPort, bufferevent * bev,TcpConnectPool * tcpConnectPool, void * arg);
    private:
        std::map<std::string, std::string> userList;
        std::map<std::string, std::string> keyList;
        CMutex LatelyOperatorTimeList_lock;
        std::map<std::string, struct timeval> LatelyOperatorTimeList;

        void insert(std::string user, std::string key); 
        bool erase(std::string key,std::string user="");
        bool reinsert(std::string user,std::string newKey);
        std::string findKey(std::string user);
        std::string findUser(std::string key);

        CMutex Sensing_Map_lock;
        std::map<std::string, SensingOperating *> SensingMap_id;
        std::map<SensingOperating *, std::string > SensingMap_sensing;
        void SensingMapAdd(std::string id, SensingOperating * sensingOperating);
        void SensingMapDel(SensingOperating * sensingOperating);
        void SensingMapDel(std::string id);
        SensingOperating * SensingMapfind(std::string id);
        std::string SensingMapfind(SensingOperating * sensingOperating);

        CMutex User_Sensing_Map_lock;
        std::map<std::string, SensingOperating *> User_Sensing_Map_user;
        std::map<SensingOperating *, std::string> User_Sensing_Map_sensing;
        std::map<std::string, httpChilent_t *> User_Sensing_Map_client;
        void AddSensing_User_Map_client(std::string user, httpChilent_t *httpChilent);
        void AddSensing_User_Map(std::string user, SensingOperating * sensingOperating);
        SensingOperating * User_Sensing_MapfindSensing(std::string user);
        httpChilent_t * User_Sensing_MapfindWebSocket(std::string user);
        std::string User_Sensing_MapfindUser(SensingOperating *sensingOperating);
        void DelSensing_User_Map(std::string user);
        void DelSensing_User_Map(SensingOperating * sensingOperating);
        void DelSensing_User_Map_noClose(std::string user);
    };
    UserListClass userList;
private:
    void login(struct httpChilent_t *httpChilent);
    void logon(struct httpChilent_t *httpChilent);

    void Register(struct httpChilent_t *httpChilent);
    void Resqonse(struct httpChilent_t *httpChilent, std::string out);
    void ResqonseError(struct httpChilent_t *httpChilent, int error_V);
    void AddSensing(struct httpChilent_t *httpChilent);
    void DelSensing(struct httpChilent_t *httpChilent);
    void GetSensingList(struct httpChilent_t *httpChilent);
    void GetSensingData(struct httpChilent_t *httpChilent);
    void GetConSensing(struct httpChilent_t *httpChilent);
};
#endif
