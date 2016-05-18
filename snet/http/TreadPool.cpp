//
//  TreadPool.cpp
//  snet
//
//  Created by San on 16/3/7.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "TreadPool.h"

void* WorkerThread::startRoutine(void *arg) {
    WorkerThread* pThread = (WorkerThread*) arg;
    pThread->working();
    return NULL;
}

void WorkerThread::start() {
    (void)pthread_create(&m_thread_id, NULL, startRoutine, this);
}

void WorkerThread::working() {
    while (true) {
        m_thread_notify.lock();
        //防止假醒 (due to signal/ENITR)
        while (m_task_list.empty()) {
            m_thread_notify.wait();
        }
        
        Task* pTask = m_task_list.front();
        m_task_list.pop_front();
        m_thread_notify.unlock();
        
        pTask->run();
        delete pTask;
//        m_task_count++;
    }
}

void WorkerThread::pushTask(Task* pTask) {
    m_thread_notify.lock();
    m_task_list.push_back(pTask);
    m_thread_notify.signal();
    m_thread_notify.unlock();
}

int ThreadPool::init(int count) {
    m_count = count;
    if (m_count <= 0)
        return -1;
    m_workers = new WorkerThread[m_count];
    for (uint32_t i = 0; i < m_count; i++) {
        m_workers[i].setIndex(i);
        m_workers[i].start();
    }
    return 0;
}

void ThreadPool::addTask(Task* pTask) {
    uint32_t index = random() % m_count;
    m_workers[index].pushTask(pTask);
}
