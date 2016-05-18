//
//  TreadPool.hpp
//  snet
//
//  Created by San on 16/3/7.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef TreadPool_h
#define TreadPool_h

#include "../stdafx.h"
#include "../MutexGuard.h"
#include "stdafx_http.h"

class Task {
public:
    Task() {}
    virtual ~Task() {}
    virtual void run() = 0;
};

class WorkerThread
{
public:
    WorkerThread():m_index(0), m_task_count(0)
    {}
    ~WorkerThread() {}
    
    static void* startRoutine(void* arg);
    void start();
    void working();
    void pushTask(Task* pTask);
    void setIndex(uint32_t index) {m_index = index;}
    
private:
    int m_index;
    int	m_task_count;
    pthread_t	m_thread_id;
    Notify m_thread_notify;
    std::list<Task*> m_task_list;
};

class ThreadPool
{
public:
    ThreadPool():m_count(0), m_workers(NULL)
    {}
    ~ThreadPool() {}
    int init(int count);
    void addTask(Task* task);
    
private:
    int m_count;
    WorkerThread* m_workers;
};

#endif /* TreadPool_h */
