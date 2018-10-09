/*************************************************************************
# > File Name: CMutex.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-03 15:02:56
# > Revise Time: 2018-10-03 22:39:23
 ************************************************************************/
#include "CMutex.hpp"
#include<iostream>
CMutex::CMutex() 
{ 
    pthread_mutex_init(&this->_mutex,NULL); 
} 
CMutex::~CMutex() 
{ 
    pthread_mutex_destroy(&this->_mutex); 
} 
int32_t CMutex::lock() 
{ 
    return pthread_mutex_lock(&_mutex); 
} 
int32_t CMutex::unlock() 
{ 
    return pthread_mutex_unlock(&this->_mutex); 
} 
int32_t CMutex::trylock() 
{ 
    return pthread_mutex_trylock(&this->_mutex); 
}
