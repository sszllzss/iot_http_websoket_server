/*************************************************************************
# > File Name: CMutex.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-03 15:04:33
# > Revise Time: 2018-10-03 15:06:26
 ************************************************************************/

#ifndef _CMUTEX_H
#define _CMUTEX_H
#include <pthread.h>
#include <iostream>
#include <stdint.h>
class CMutex
{
public:
    CMutex();
    ~CMutex();
    int32_t lock();
    int32_t unlock();
    int32_t trylock();
private:
    pthread_mutex_t _mutex;

};
#endif
