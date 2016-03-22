//
//  IOLoop.h
//  snet
//
//  Created by San on 15/11/7.
//  Copyright © 2015年 ___SAN___. All rights reserved.
//

#ifndef __snet__IOLoop__
#define __snet__IOLoop__

#include "stdafx.h"
#include "Callback.h"

class MutexGuard;
class IOEvent;
#ifdef _WIN32
class IOSelect;
#define Poller IOSelect
#else
class IOKevent;
#define Poller IOKevent
#endif

class IOLoop
{
public:
    IOLoop();
    ~IOLoop();
    
    int start_loop();
    int add_handle(IOEvent* ev);
    int update_handle(IOEvent* ev, int next_events);
    int remove_handle(IOEvent* ev);
    
    int add_handle(const Function_t& func);
    TimeItem_t* add_timer(uint64_t timeout, const Function_t& callback);
    int remove_timer(const TimeItem_t* timer);
    
private:
    void loop();
    
private:
    Poller* m_poller;
    bool m_started;
    
    typedef std::vector<IOEvent*> EventList;
    EventList m_activeEventList;
    typedef std::vector<TimeItem_t*> TimeEventList_t;
    TimeEventList_t m_timeEventList;
    typedef std::vector<Function_t> PostEventList_t;
    PostEventList_t m_postEventList;
};

#endif /* __snet__IOLoop__ */
