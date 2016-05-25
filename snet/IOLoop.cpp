//
//  IOLoop.cpp
//  snet
//
//  Created by San on 16/2/25.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifdef _WIN32
#include "IOSelect.h"
#else
#include "IOKevent.h"
#endif
#include <algorithm>
#include "IOLoop.h"
#include "IOEvent.h"
#include "Log.h"

IOLoop::IOLoop()
:m_started(false),m_poller(new Poller())
{}

IOLoop::~IOLoop() {}

int IOLoop::start_loop() {
    m_started = true;
    loop();
    return 0;
}

int IOLoop::add_handle(IOEvent* ev) {
    return m_poller->add_handle(ev);
}
int IOLoop::update_handle(IOEvent* ev, int next_events) {
    return m_poller->update_handle(ev, next_events);
}
int IOLoop::remove_handle(IOEvent* ev) {
    return m_poller->remove_handle(ev);
}

int IOLoop::add_handle(const Function_t& func) {
    m_postEventList.push_back(func);
    return 0;
}

TimeItem_t* IOLoop::add_timer(uint64_t timeout, const Function_t& callback) {
    TimeItem_t* pTime = new TimeItem_t;
    pTime->callback = callback;
    pTime->timeout = timeout;
    pTime->next_tick = get_tick() + timeout;
    m_timeEventList.push_back(pTime);
    return pTime;
}
int IOLoop::remove_timer(const TimeItem_t* timer) {
    if(timer != nullptr) {
        auto it = std::find(m_timeEventList.begin(), m_timeEventList.end(), timer);
        if(it != m_timeEventList.end()) {
            m_timeEventList.erase(it);
        }
    }
    return 0;
}

void IOLoop::loop() {
    for(;;) {
        m_activeEventList.clear();
        int ret = m_poller->processEvent(&m_activeEventList);
        //LOG_STDOUT("Loop: Start run EventHandler");
        for(auto it = m_activeEventList.begin(); it != m_activeEventList.end(); it++) {
            IOEvent* ev = *it;
            ev->handleEvent((void*)&ret);
        }
        TimeEventList_t timeObj;
        timeObj.swap(m_timeEventList);
        uint64_t now = get_tick();
        for (TimeItem_t* it : timeObj) {
            if (now >= it->next_tick) {
                it->callback((void*)&now);
                delete it;
            }
            else {
                m_timeEventList.push_back(it);
            }
        }
        PostEventList_t postFunList;
        postFunList.swap(m_postEventList);
        //LOG_STDOUT("Loop: Start run FuncHandler");
        for (Function_t& it : postFunList){
            it(NULL);
        }
    }
}