/*************************************************************************
# > File Name: TcpConnectPool.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-17 12:20:36
# > Revise Time: 2018-10-17 12:27:23
 ************************************************************************/

#ifndef _TCPCONNECTPOOL_H
#define _TCPCONNECTPOOL_H
#include "Http_websocket/include/evbase_threadpool.h"
class TcpConnectPool
{
private:
    evbase_threadpool_t *tcp_ev_threadpool;        
};
#endif
