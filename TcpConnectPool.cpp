/*************************************************************************
# > File Name: TcpConnectPool.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-17 12:20:30
# > Revise Time: 2018-10-22 17:18:11
 ************************************************************************/

#include<iostream>
#include"TcpConnectPool.h"
#include "string.h"
#define TCPCONNECTPOOL_THREADPOOL_MIN_NUM 10 
#define TCPCONNECTPOOL_BASE_MAX_MONITOR_NUM 10 
#define TCPCONNECTPOOL_TIMEOUT_CHECK_PERIOD 100 // 100ms
void TcpConnectPool_bufferev_read_cb(bufferevent * bev, void *arg)
{
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    TcpConnectPool::IpPort_t *ipPort = NULL;
    ipPort = tcpConnectPool->GetIpPort(bev); 
    tcpConnectPool->AddTreadCbFun(TcpConnectPool::THREADPOOL_FUN_TYPE::READ_CB,*ipPort,bev);
    free(ipPort);

}
void TcpConnectPool_bufferev_write_cb(bufferevent * bev, void *arg)
{
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    TcpConnectPool::IpPort_t *ipPort = NULL;
    ipPort = tcpConnectPool->GetIpPort(bev); 
    tcpConnectPool->AddTreadCbFun(TcpConnectPool::THREADPOOL_FUN_TYPE::WIRTER_CB,*ipPort,bev);
    free(ipPort);
}
void TcpConnectPool_bufferev_event_cb(struct bufferevent *  bev, short events, void * arg)
{
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    std::string err_str;
    TcpConnectPool::IpPort_t *ipPort = NULL;
    ipPort = tcpConnectPool->GetIpPort(bev); 
    if(events == BEV_EVENT_ERROR)
    {
        int err = EVUTIL_SOCKET_ERROR();
        err_str = ipPort->ip + std::to_string(ipPort->port) + "] ";
        err_str += "Got an error"+std::to_string(err) +" ("+  evutil_socket_error_to_string(err) +") on the Connect.connect close.";
        tcpConnectPool->AddThredErrorCbFun(TcpConnectPool::ErrorType::ERROR_CONNECT, err_str);
    }
    else if(events == BEV_EVENT_EOF)
    {

    }
    else
    {
        err_str = ipPort->ip + std::to_string(ipPort->port) + "] ";
        err_str += "Unkowon error!";
        tcpConnectPool->AddThredErrorCbFun(TcpConnectPool::ErrorType::ERROR_UNKNOWO, err_str);
    }
    tcpConnectPool->DelTcpCilent(bev);
    bufferevent_lock(bev);
    bufferevent_free(bev);
    tcpConnectPool->AddTreadDisConnectCbFun(*ipPort);
    free(ipPort);
}
void *TcpConnectPool_threadPool_fun(void * arg)
{
    if(arg == NULL)
        return NULL;
    TcpConnectPool::Threadpool_fun_arg_t * fun_arg = (TcpConnectPool::Threadpool_fun_arg_t *)arg;
    if(fun_arg->arg ==  NULL) 
        return NULL;
    TcpConnectPool::TcpRead_cb_t read_cb = (TcpConnectPool::TcpRead_cb_t)fun_arg->fun;
    TcpConnectPool::Thread_fun_arg_cb_arg_t *arg_cb = (TcpConnectPool::Thread_fun_arg_cb_arg_t *)fun_arg->arg;
    TcpConnectPool::TcpWirter_cb_t wirter_cb = (TcpConnectPool::TcpWirter_cb_t)fun_arg->fun;
    TcpConnectPool::TcpConnect_cb_t connect_cb = (TcpConnectPool::TcpConnect_cb_t)fun_arg->fun;
    TcpConnectPool::TcpDisConnect_cb_t disconnect_cb = (TcpConnectPool::TcpDisConnect_cb_t)fun_arg->fun;
    TcpConnectPool::Thread_fun_arg_disconnect_cb_arg_t *arg_disconnect_cb = (TcpConnectPool::Thread_fun_arg_disconnect_cb_arg_t *)fun_arg->arg;
    TcpConnectPool::TcpError_cb_t error_cb = (TcpConnectPool::TcpError_cb_t)fun_arg->fun;
    TcpConnectPool::Thread_fun_arg_error_cb_arg_t *arg_error_cb = (TcpConnectPool::Thread_fun_arg_error_cb_arg_t *)fun_arg->arg;
    std::string err_str;
    if(read_cb == NULL)
    {
        free(fun_arg->arg);
        free(fun_arg);
        return NULL;
    }
    switch(fun_arg->fun_type)
    {
    case TcpConnectPool::READ_CB:
        bufferevent_lock(arg_cb->bev);
        read_cb(arg_cb->ipPort, arg_cb->bev, arg_cb->tcpConnectPool);
        bufferevent_unlock(arg_cb->bev);
        break;
    case TcpConnectPool::WIRTER_CB:
        bufferevent_lock(arg_cb->bev);
        wirter_cb(arg_cb->ipPort, arg_cb->bev, arg_cb->tcpConnectPool);
        bufferevent_unlock(arg_cb->bev);
        break;
    case TcpConnectPool::CONNECT_CB:
        bufferevent_lock(arg_cb->bev);
        connect_cb(arg_cb->ipPort, arg_cb->bev, arg_cb->tcpConnectPool);
        bufferevent_unlock(arg_cb->bev);
        break;
    case TcpConnectPool::DISCONNECT_CB:
        disconnect_cb(arg_disconnect_cb->ipPort, arg_disconnect_cb->tcpConnectPool);
        break;
    case TcpConnectPool::ERROR_CB:
        if(arg_error_cb->err_str != NULL)
        {
            err_str = arg_error_cb->err_str;
            free(arg_error_cb->err_str);
        }
        else
        {
            err_str = "Unknown error!";
        }
        error_cb(arg_error_cb->tcpConnectPool, arg_error_cb->error,err_str);
        break;
    }
    free(fun_arg->arg);
    free(fun_arg);
    return  NULL;
}
void *TcpConnectPool_thread_fun(void * arg)
{
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    event_base_dispatch(tcpConnectPool->listener_base);
    tcpConnectPool->lock();
    event_base_free(tcpConnectPool->listener_base);
    evconnlistener_free(tcpConnectPool->listener);
    tcpConnectPool->listener_base = NULL;
    tcpConnectPool->listener = NULL;
    tcpConnectPool->error = TcpConnectPool::ErrorType::ERROR_BASE_BREAK;
    tcpConnectPool->unlock();
    return NULL;
}
void TcpConnectPool_listener_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * addr, int socklen, void * arg)
{
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    listener = (struct evconnlistener *)listener;
    socklen = (int)socklen;
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in * addr_in = (struct sockaddr_in *)addr;
    inet_ntop(AF_INET, &addr_in->sin_addr, ip, INET_ADDRSTRLEN);
    event_base * base = evbase_threadpool_add_event(tcpConnectPool->tcp_ev_threadpool);
    if(base != NULL)
    {
        struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        if(bev != NULL)
        {
            TcpConnectPool::IpPort_t ipPort;
            ipPort.port = ntohs(addr_in->sin_port);
            strcpy(ipPort.ip, ip);
            bufferevent_data_cb read_cb = NULL;
            bufferevent_data_cb write_cb = NULL;
            if(tcpConnectPool->read_cb != NULL)
                read_cb = TcpConnectPool_bufferev_read_cb;
            if(tcpConnectPool->wirter_cb != NULL)
                write_cb = TcpConnectPool_bufferev_write_cb;
            bufferevent_setcb(bev, read_cb, write_cb, TcpConnectPool_bufferev_event_cb, tcpConnectPool);
            bufferevent_enable(bev, EV_READ| EV_WRITE);
            tcpConnectPool->AddTcpClient(ip,ntohs(addr_in->sin_port),bev);
            tcpConnectPool->AddTreadCbFun(TcpConnectPool::THREADPOOL_FUN_TYPE::CONNECT_CB, ipPort, bev);
        }
        else
        {
            evbase_threadpool_close_event(tcpConnectPool->tcp_ev_threadpool, base);
            tcpConnectPool->lock();
            if(tcpConnectPool->error_cb  != NULL)
            {
                std::string err_str = ip;
                err_str += ":" + std::to_string(ntohs(addr_in->sin_port)) + "] bufferevent_socket_new  fail!";
                tcpConnectPool->unlock();
                tcpConnectPool->error = TcpConnectPool::ErrorType::ERROR_BUFFEREVENT;
                tcpConnectPool->AddThredErrorCbFun(tcpConnectPool->error , err_str);
                return;
            }
            tcpConnectPool->unlock();
            return;
        }
    }
    else
    {
        tcpConnectPool->lock();
        if(tcpConnectPool->error_cb  != NULL)
        {
            std::string err_str = ip;
            err_str += ":" + std::to_string(ntohs(addr_in->sin_port)) + "] evbase_threadpool_add_event fail!";
            tcpConnectPool->unlock();
            tcpConnectPool->error = TcpConnectPool::ErrorType::ERROR_ADD_EVENT;
            tcpConnectPool->AddThredErrorCbFun(tcpConnectPool->error , err_str);
            return;
        }
        tcpConnectPool->unlock();
    }

}
void TcpConnectPool_accept_error_cb(struct evconnlistener *listener, void * arg)
{
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    struct event_base *base = evconnlistener_get_base(listener);
    event_base_loopexit(base , NULL);
    tcpConnectPool->error = TcpConnectPool::ErrorType::ERROR_ACCEPT;
}
void TcpConnectPool_time_inspect_cb(evutil_socket_t fd, short events, void *arg)
{
    fd = (evutil_socket_t)fd;
    events = (short)events;
    TcpConnectPool * tcpConnectPool = (TcpConnectPool *)arg;
    tcpConnectPool->lock();
    if(tcpConnectPool->listener == NULL || tcpConnectPool->listener_base == NULL)
    {
        if(tcpConnectPool->error_cb != NULL)
        {
            std::string err_str;
            if(tcpConnectPool->error == TcpConnectPool::ErrorType::ERROR_ACCEPT)
            {
                int err = EVUTIL_SOCKET_ERROR();
                err_str = "Got an error"+std::to_string(err) +" ("+  evutil_socket_error_to_string(err) +") on the listener. Shutting down.";
                tcpConnectPool->unlock();
                tcpConnectPool->error_cb(tcpConnectPool,tcpConnectPool->error,  err_str);
                return;
            }
            else if(tcpConnectPool->error == TcpConnectPool::ErrorType::ERROR_BASE_BREAK)
            {
                err_str = "listener_thread break!";
                tcpConnectPool->unlock();
                tcpConnectPool->error_cb(tcpConnectPool,TcpConnectPool::ErrorType::ERROR_BASE_BREAK ,  err_str);
                return;
            }
            else
            {
                err_str = "Unkowon error!";
                tcpConnectPool->unlock();
                tcpConnectPool->error_cb(tcpConnectPool,TcpConnectPool::ErrorType::ERROR_BASE_BREAK ,  err_str);
                return;

            }
        }
    }
    tcpConnectPool->unlock();
}
void TcpConnectPool::AddThredErrorCbFun( ErrorType error, std::string err_str)
{
                TcpConnectPool::Threadpool_fun_arg_t *fun_arg;
                fun_arg = (TcpConnectPool::Threadpool_fun_arg_t *)malloc(sizeof(TcpConnectPool::Threadpool_fun_arg_t));
                if(fun_arg)
                {
                    fun_arg->fun = (void *)this->error_cb;
                    if(fun_arg->fun==NULL)
                    {
                        free(fun_arg);
                        return;
                    }
                    fun_arg->fun_type = ERROR_CB;
                    TcpConnectPool::Thread_fun_arg_error_cb_arg_t *cb_arg;
                    cb_arg = (TcpConnectPool::Thread_fun_arg_error_cb_arg_t *)malloc(sizeof(TcpConnectPool::Thread_fun_arg_error_cb_arg_t));
                    if(cb_arg)
                    {
                        fun_arg->arg = cb_arg;
                        cb_arg->tcpConnectPool = this;
                        cb_arg->error = error;
                        cb_arg->err_str =  (char *)malloc(err_str.size());
                        if(cb_arg->err_str != NULL)
                            strcpy(cb_arg->err_str, err_str.c_str());
                        threadpool_add(evbase_threadpool_get_threadpool(this->tcp_ev_threadpool), TcpConnectPool_threadPool_fun, fun_arg);
                    }
                }


    
}
void TcpConnectPool::AddTreadDisConnectCbFun(IpPort_t &ipPort)
{
                TcpConnectPool::Threadpool_fun_arg_t *fun_arg;
                fun_arg = (TcpConnectPool::Threadpool_fun_arg_t *)malloc(sizeof(TcpConnectPool::Threadpool_fun_arg_t));
                if(fun_arg)
                {
                    fun_arg->fun = (void *)this->disConnect_cb;
                    if(fun_arg->fun==NULL)
                    {
                        free(fun_arg);
                        return;
                    }
                    fun_arg->fun_type = DISCONNECT_CB;
                    TcpConnectPool::Thread_fun_arg_disconnect_cb_arg_t *cb_arg;
                    cb_arg = (TcpConnectPool::Thread_fun_arg_disconnect_cb_arg_t *)malloc(sizeof(TcpConnectPool::Thread_fun_arg_disconnect_cb_arg_t));
                    if(cb_arg)
                    {
                        fun_arg->arg = cb_arg;
                        cb_arg->tcpConnectPool = this;
                        cb_arg->ipPort.port = ipPort.port;
                        strcpy(cb_arg->ipPort.ip, ipPort.ip);
                        threadpool_add(evbase_threadpool_get_threadpool(this->tcp_ev_threadpool), TcpConnectPool_threadPool_fun, fun_arg);
                    }
                }


    
}
void TcpConnectPool::AddTreadCbFun(TcpConnectPool::THREADPOOL_FUN_TYPE type, IpPort_t & ipPort, bufferevent * bev)
{
                TcpConnectPool::Threadpool_fun_arg_t *fun_arg;
                fun_arg = (TcpConnectPool::Threadpool_fun_arg_t *)malloc(sizeof(TcpConnectPool::Threadpool_fun_arg_t));
                if(fun_arg)
                {
                    switch(type)
                    {
                    case READ_CB:
                        fun_arg->fun = (void *)this->read_cb;
                        break;
                    case WIRTER_CB:
                        fun_arg->fun = (void *)this->wirter_cb;
                        break;
                    case CONNECT_CB:
                        fun_arg->fun = (void *)this->connect_cb;
                        break;
                    case DISCONNECT_CB:
                    case ERROR_CB:
                        free(fun_arg);
                        return;
                    }
                    if(fun_arg->fun==NULL)
                    {
                        free(fun_arg);
                        return;
                    }
                    fun_arg->fun_type = type;
                    TcpConnectPool::Thread_fun_arg_cb_arg_t *cb_arg;
                    cb_arg = (TcpConnectPool::Thread_fun_arg_cb_arg_t *)malloc(sizeof(TcpConnectPool::Thread_fun_arg_cb_arg_t));
                    if(cb_arg)
                    {
                        fun_arg->arg = cb_arg;
                        cb_arg->bev = bev; 
                        cb_arg->tcpConnectPool = this;
                        cb_arg->ipPort.port = ipPort.port;
                        strcpy(cb_arg->ipPort.ip, ipPort.ip);
                        threadpool_add(evbase_threadpool_get_threadpool(this->tcp_ev_threadpool), TcpConnectPool_threadPool_fun, fun_arg);
                    }
                }

}
TcpConnectPool::TcpConnectPool(event_base * base,  int port, TcpConnectPool::TcpRead_cb_t read_cb,TcpConnectPool::TcpConnect_cb_t connect_cb,TcpConnectPool::TcpDisConnect_cb_t disConnect_cb)
{
    std::string err_str;
    do{
        this->tcp_ev_threadpool = NULL;
        this->listener = NULL;
        this->listener_base = NULL;
        this->listener_thread = 0;
        this->error_cb = NULL;
        this->wirter_cb = NULL;
        this->read_cb = read_cb;
        this->connect_cb = connect_cb;
        this->disConnect_cb = disConnect_cb;
        this->error = TcpConnectPool::ErrorType::NOT_ERROE;
        this->listener_base = event_base_new();
        if(this->listener_base == NULL)
        {
            err_str = "event_base_new fail!";
            break;
        }
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        this->tcp_ev_threadpool = evbase_threadpool_new(TCPCONNECTPOOL_BASE_MAX_MONITOR_NUM,TCPCONNECTPOOL_THREADPOOL_MIN_NUM);
        if(this->tcp_ev_threadpool == NULL)
        {
            err_str = "evbase_threadpool_new fail:"; 
            event_base_free(this->listener_base);    
            break;
        }
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
        this->listener = evconnlistener_new_bind(listener_base, TcpConnectPool_listener_cb, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(sin));
        if(this->listener == NULL)
        {
            err_str = "evconnlistener_new_bind fail:"; 
            event_base_free(this->listener_base);    
            evbase_threadpool_destroy(this->tcp_ev_threadpool);
            break;
        }
        evconnlistener_set_error_cb(this->listener, TcpConnectPool_accept_error_cb);
        int rec = pthread_create(&listener_thread, &attr, TcpConnectPool_thread_fun, this);
        if(rec != 0)
        {
            err_str = "pthread_create fail:"; 
            err_str += strerror(rec);
            event_base_free(this->listener_base);    
            evbase_threadpool_destroy(this->tcp_ev_threadpool);
            evconnlistener_free(this->listener);
            break;
        }
        /*检查定时器 */
        event_assign(&ev_time_inspect, base, -1, EV_PERSIST, TcpConnectPool_time_inspect_cb, this);
        struct timeval tv;
        evutil_timerclear(&tv);
        tv.tv_sec = 0;
        tv.tv_usec = 1000*TCPCONNECTPOOL_TIMEOUT_CHECK_PERIOD; //200ms
        event_add(&ev_time_inspect, &tv);
    }while(0);
    if(!err_str.empty())
        throw err_str;
}
TcpConnectPool::~TcpConnectPool()
{
    lock();
    if(this->listener != NULL)
        evconnlistener_free(this->listener);
    if(this->listener_base != NULL)
    {
        event_base_loopexit(this->listener_base, NULL);
        event_base_free(this->listener_base);    
    }
    if(this->listener_thread != 0)
    {
        event_del(&ev_time_inspect);
        pthread_join(this->listener_thread, NULL);
    }
    if(this->tcp_ev_threadpool != NULL)
        evbase_threadpool_destroy(this->tcp_ev_threadpool);
}
void TcpConnectPool::Set_Wirter_cb(TcpConnectPool::TcpWirter_cb_t wirter_cb)
{
    lock();
    this->wirter_cb = wirter_cb;
    unlock();
}
void TcpConnectPool::Set_Error_cb(TcpError_cb_t error_cb)
{
    lock();
    this->error_cb = error_cb;
    unlock();
}
void TcpConnectPool::AddTcpClient(std::string ip,int port, bufferevent * bev)
{
    std::string Str = ip + ":" + std::to_string(port);
    bevMap_lock.lock();
    bevMap_bev.insert(std::pair<bufferevent *,std::string>(bev,Str));
    bevMap_ip.insert(std::pair<std::string, bufferevent *>(Str,bev));
    bevMap_lock.unlock();
}
void TcpConnectPool::DelTcpCilent(std::string ip, int port)
{
    std::string Str = ip + ":" + std::to_string(port);
    bevMap_lock.lock();
    std::map<std::string, bufferevent *>::iterator i= bevMap_ip.find(Str);
    if(i != bevMap_ip.end())
    {
        bevMap_bev.erase(bevMap_bev.find(i->second));
        bevMap_ip.erase(i);
    }
    bevMap_lock.unlock();
}
void TcpConnectPool::DelTcpCilent(bufferevent *bev)
{    
    bevMap_lock.lock();
    std::map<bufferevent *, std::string>::iterator i = bevMap_bev.find(bev);
    if(i != bevMap_bev.end())
    {
        bevMap_ip.erase(bevMap_ip.find(i->second));
        bevMap_bev.erase(i);
    }
    bevMap_lock.unlock();
}
TcpConnectPool::IpPort_t * TcpConnectPool::GetIpPort(struct bufferevent * bev)
{
    IpPort_t *ipPort=NULL;
    bevMap_lock.lock();
    std::map<bufferevent *, std::string>::iterator i = bevMap_bev.find(bev);
    if(i != bevMap_bev.end())
    {
        ipPort = (IpPort_t *)malloc(sizeof(IpPort_t));
        memset(ipPort,0, sizeof(IpPort_t));
        if(ipPort != NULL)
        {
            std::string strs = i->second;
            size_t pos = strs.find(":");
            size_t size = strs.size();
            memcpy(ipPort->ip, strs.substr(0,pos).c_str(), pos);
            ipPort->port = atoi(strs.substr(pos+1,size).c_str());
        }
    }
    bevMap_lock.unlock();
    return ipPort;
}
struct bufferevent *TcpConnectPool::GetBev(TcpConnectPool::IpPort_t & ipPort)
{
    bufferevent * bev = NULL;
    std::string Str = ipPort.ip; 
    Str += ":" + std::to_string(ipPort.port);
    bevMap_lock.lock();
    std::map<std::string, bufferevent *>::iterator i = bevMap_ip.find(Str);
    if(i != bevMap_ip.end())
    {
        bev  = i->second;
    }
    bevMap_lock.unlock();
    return bev;
}
