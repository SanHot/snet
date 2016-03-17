//
//  MutexGuard.hpp
//  snet
//
//  Created by San on 16/3/1.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__MutexGuard_h
#define __snet__MutexGuard_h

#include "stdafx.h"
#include "Util.h"

#ifdef WIN32
#define MUTEXOBJECT_T CRITICAL_SECTION
#else
#define MUTEXOBJECT_T pthread_mutex_t
#endif

#ifdef MUTEX_OWN
typedef MutexGuard MUTEXGUARD_T;
typedef Mutex MUTEX_T;
#else
typedef std::unique_lock<std::mutex> MUTEXGUARD_T;
typedef std::mutex MUTEX_T;
#endif

class Mutex
{
public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
    MUTEXOBJECT_T *getMutex() { return &m_mutex;}
    
private:
    DISALLOW_EVIL_CONSTRUCTORS(Mutex);
#ifdef WIN32
    uint32_t m_threadId;
#else
    pthread_t m_threadId;
#endif
    MUTEXOBJECT_T m_mutex;
};

class MutexGuard
{
public:
    MutexGuard(Mutex &mutex): m_mutex(mutex) {m_mutex.lock();}
    ~MutexGuard() {m_mutex.unlock();}
private:
    DISALLOW_EVIL_CONSTRUCTORS(MutexGuard);
    Mutex &m_mutex;
};

#define MutexGuard(x) error "Missing MutexGuard object name"

class Notify
{
public:
    Notify() {
        pthread_mutexattr_init(&m_attr);
        pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &m_attr);
        pthread_cond_init(&m_cond, NULL);
    }
    ~Notify() {
        pthread_mutexattr_destroy(&m_attr);
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    
    void lock() {pthread_mutex_lock(&m_mutex);}
    void unlock() {pthread_mutex_unlock(&m_mutex);}
    void wait() {pthread_cond_wait(&m_cond, &m_mutex);}
    void signal() {pthread_cond_signal(&m_cond);}
    
private:
    pthread_mutex_t m_mutex;
    pthread_mutexattr_t m_attr;
    pthread_cond_t m_cond;
};

#endif /* MutexGuard_h */
