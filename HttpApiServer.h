/*************************************************************************
# > File Name: /home/sszl/Src/IotHttpWebsocketServer/HttpApiServer.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-09 18:04:59
# > Revise Time: 2018-10-22 11:00:39
 ************************************************************************/

#ifndef _HTTPAPISERVER_H
#define _HTTPAPISERVER_H
#include "Http_websocket/include/evbase_threadpool.h" 
#include "Http_websocket/include/ev_httpd.h"
#include "database.h"
#include <signal.h>
#include <map>
#include "TcpConnectPool.h"
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
class HttpApiServer : private CMutex
{
    friend void *HttpApiServer_httplisten_thread(void *arg);
    friend void ev_time_inspect_cb(evutil_socket_t fd, short events, void *arg);
    friend void httpHandler_cb(struct httpChilent_t * httpChilent);
    friend void ws_read_cb(struct httpChilent_t * httpChilent);
public:
    HttpApiServer(u_short port);
    ~HttpApiServer();
    void start_loop();
    void end_loop();
public:
    /* 异常 */
    HttpApiServerException error;
private:
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
        friend void ev_time_inspect_cb(evutil_socket_t fd, short events, void *arg);
    private:
        std::map<std::string, std::string> userList;
        std::map<std::string, std::string> keyList;
        std::map<std::string, struct timeval> LatelyOperatorTimeList;
        void insert(std::string user, std::string key); 
        bool erase(std::string key,std::string user="");
        bool reinsert(std::string user,std::string newKey);
        std::string findKey(std::string user);
        std::string findUser(std::string key);
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
