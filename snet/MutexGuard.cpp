//
//  MutexGuard.cpp
//  snet
//
//  Created by San on 16/3/1.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "MutexGuard.h"

Mutex::Mutex()
:m_threadId(0)
{
#ifdef _MSC_VER
    InitializeCriticalSection(&m_mutex);
#else
    pthread_mutex_init(&m_mutex, NULL);
#endif
}
Mutex::~Mutex()
{
#ifdef _MSC_VER
    DeleteCriticalSection(&m_mutex);
#else
    pthread_mutex_destroy(&m_mutex);
#endif
}

void Mutex::lock() {
#ifdef _MSC_VER
    EnterCriticalSection(&m_mutex);
    //m_threadId = static_cast<uint32_t>(GetCurrentThreadId());
#else
    pthread_mutex_lock(&m_mutex);
    //m_threadId = pthread_self();
#endif
}

void Mutex::unlock() {
    m_threadId = 0;
#ifdef _MSC_VER
    LeaveCriticalSection(&m_mutex);
#else
    pthread_mutex_unlock(&m_mutex);
#endif
}
