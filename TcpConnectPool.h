/*************************************************************************
# > File Name: /home/sszl/Src/IotHttpWebsocketServer/TcpConnectPool.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-17 12:20:36
# > Revise Time: 2018-11-01 11:25:52
 ************************************************************************/

#ifndef _TCPCONNECTPOOL_H
#define _TCPCONNECTPOOL_H
#include "Http_websocket/include/evbase_threadpool.h"
#include <event2/listener.h>
#include <map>
#include "CMutex.hpp"
class TcpConnectPool : private CMutex
{
friend void *TcpConnectPool_thread_fun(void * arg);
friend void TcpConnectPool_time_inspect_cb(evutil_socket_t fd, short events, void *arg);
friend void TcpConnectPool_listener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * addr, int socklen, void * arg);
friend void TcpConnectPool_accept_error_cb(struct evconnlistener *listener, void * arg);
friend void TcpConnectPool_bufferev_event_cb(struct bufferevent *  bev, short events, void * arg);
friend void TcpConnectPool_bufferev_read_cb(bufferevent * bev, void *arg);
friend void TcpConnectPool_bufferev_write_cb(bufferevent * bev, void *arg);
friend void *TcpConnectPool_threadPool_fun(void * arg);
friend void *TcpConnectPool_colesConnect(void * arg);
private:
    evbase_threadpool_t *tcp_ev_threadpool;        
    struct evconnlistener * listener;
    struct event_base * listener_base;
    pthread_t listener_thread;
    CMutex bevMap_lock;
    event ev_time_inspect;
    std::map<std::string , bufferevent *> bevMap_ip;
    std::map<bufferevent *, std::string> bevMap_bev;

    void * cb_arg;
private:
    void AddTcpClient(std::string ip , int port, bufferevent * bev);
    void DelTcpCilent(std::string ip, int port);
    void DelTcpCilent(bufferevent *bev);
public:
    typedef struct 
    {
        char ip[16];
        int port;
    }IpPort_t;
    enum ErrorType 
    {
        NOT_ERROE = 0,
        ERROR_ADD_EVENT = -1,
        ERROR_ACCEPT = -2,
        ERROR_BASE_BREAK = -3,
        ERROR_BUFFEREVENT = -4,
        ERROR_CONNECT = -5,
        ERROR_UNKNOWO = -6,
    };
    enum THREADPOOL_FUN_TYPE
    {
        READ_CB = 1,
        DISCONNECT_CB,
        CONNECT_CB,
        WIRTER_CB,
        ERROR_CB
    };
    typedef struct
    {
        IpPort_t  ipPort;
        bufferevent * bev;
        TcpConnectPool * tcpConnectPool;
    }Thread_fun_arg_cb_arg_t;
    typedef struct
    {
        TcpConnectPool * tcpConnectPool;
        ErrorType error;
        char *  err_str;
    }Thread_fun_arg_error_cb_arg_t;
    typedef struct
    {
        void *fun;
        void* arg;
        THREADPOOL_FUN_TYPE fun_type;
    }Threadpool_fun_arg_t;
    ErrorType error;
    typedef void (*TcpRead_cb_t)(IpPort_t & ipPort,bufferevent * buff, TcpConnectPool * tcpConnectPool ,    void *arg);
    typedef void (*TcpWirter_cb_t)(IpPort_t & ipPort,bufferevent * buff, TcpConnectPool * tcpConnectPool, void *arg);
    typedef void (*TcpDisConnect_cb_t)(IpPort_t & ipPort,bufferevent * evb, TcpConnectPool * tcpConnectPool, void *arg);
    typedef void (*TcpConnect_cb_t)(IpPort_t & ipPort, bufferevent * evb, TcpConnectPool * tcpConnectPool, void *arg);
    typedef void (*TcpError_cb_t)(TcpConnectPool * tcpConnectPool, ErrorType error, std::string & err_str, void *arg);
public:
    TcpConnectPool(event_base * base, int port, void * cb_arg,TcpConnectPool::TcpRead_cb_t read_cb, TcpConnect_cb_t disConnect_cb = NULL, TcpDisConnect_cb_t connect_cb = NULL);
    ~TcpConnectPool();
     IpPort_t * GetIpPort(struct bufferevent * bev);
     struct bufferevent *GetBev(IpPort_t & ipPort);
     void Set_Error_cb(TcpError_cb_t error_cb);
     void Set_Wirter_cb(TcpWirter_cb_t error_cb);
     void ColesConnect(bufferevent * bev);
     void ColesConnect(IpPort_t & ipPort);
     void ColesConnect(std::string ip, int port);
     threadpool_t * GetThreadpool();
     void WriteData(bufferevent *bev,const char *buff, size_t size);
     void WriteData(IpPort_t & ipPort,const char *buff, size_t size);
     void WriteData(std::string ip, int port,const char *buff, size_t size);

     static std::string ErrorTypeToString(ErrorType error);
private:
    TcpRead_cb_t read_cb;
    TcpWirter_cb_t wirter_cb;
    TcpDisConnect_cb_t disConnect_cb;
    TcpConnect_cb_t connect_cb;
    TcpError_cb_t error_cb;
    void AddTreadCbFun(THREADPOOL_FUN_TYPE type, IpPort_t & ipPort, bufferevent * bev);
    void AddThredErrorCbFun( ErrorType error, std::string err_str);
    void WriteDataNoCheck(bufferevent *bev,const char *buff, size_t size);
};
#endif
