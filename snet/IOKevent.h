//
//  IOKevent.h
//  snet
//
//  Created by San on 15/8/19.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#ifndef __snet__IOKevent__
#define __snet__IOKevent__

#include "stdafx.h"

#define _EPOLLIN  0x001
#define _EPOLLPRI  0x002
#define _EPOLLOUT  0x004
#define _EPOLLERR  0x008
#define _EPOLLHUP  0x010
#define _EPOLLRDHUP  0x2000
#define _EPOLLONESHOT  (1 << 30)
#define _EPOLLET  (1 << 31)

class IOEvent;
class IOKevent
{
public:
    IOKevent();
    ~IOKevent();
    
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
    int m_kqfd;
    bool m_running;
    //事件队列
    typedef std::map<int, IOEvent*> EventMap;
    EventMap m_event_map;
};

#endif /* defined(__snet__IOKevent__) */
