//
//  IOSelect.h
//  snet
//
// Created by san on 16/3/22.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__IOSELECT_H
#define __snet__IOSELECT_H

#include "stdafx.h"
#include "MutexGuard.h"

class IOEvent;
class IOSelect
{
public:
    IOSelect();
    ~IOSelect();

public:
    typedef std::vector<IOEvent*> EventList;
    int processEvent(EventList* activeList);
    //添加SOCKET事件
    int add_handle(IOEvent* ev);
    //更新SOCKET事件
    int update_handle(IOEvent* ev, int next_events);
    //移除SOCKET事件
    int remove_handle(IOEvent* ev);

public:
    bool isPollReading(int fd);
    bool isPollWriting(int fd);

private:
    //事件句柄
    fd_set	m_read_set;
    fd_set	m_write_set;
    fd_set	m_error_set;
    bool m_running;
    //事件队列
    typedef std::map<int, IOEvent*> EventMap;
    EventMap m_event_map;
    MUTEX_T m_mtx;
};


#endif //SNET_IOSELECT_H
