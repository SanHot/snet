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

class MutexGuard;
class IOEvent;
class IOKevent;
#define Poller IOKevent

class IOLoop
{
public:
    typedef std::function<void (void*)> Function_t;
    
    IOLoop();
    ~IOLoop();
    
    int start_loop();
    int add_handle(const Function_t& func);
    int add_handle(IOEvent* ev);
    int update_handle(IOEvent* ev, int next_events);
    int remove_handle(IOEvent* ev);
    
private:
    void loop();
    
private:
    Poller* m_poller;
    bool m_started;
    
    typedef std::vector<IOEvent*> EventList;
    EventList m_activeEventList;
    typedef std::vector<Function_t> PostEvent;
    PostEvent m_postEventList;
};

#endif /* __snet__IOLoop__ */
