//
//  IOLoop.cpp
//  snet
//
//  Created by San on 16/2/25.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "IOLoop.h"
#include "IOEvent.h"
#include "IOKevent.h"
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

int IOLoop::add_handle(const Function_t& func) {
    m_postEventList.push_back(func);
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

void IOLoop::loop() {
    for(;;) {
        m_activeEventList.clear();
        int ret = m_poller->processEvent(&m_activeEventList);
        //LOG_STDOUT("Loop: Start run EventHandler");
        for(auto it = m_activeEventList.begin(); it != m_activeEventList.end(); it++) {
            IOEvent* ev = *it;
            ev->handleEvent((void*)&ret);
        }
        PostEvent postFun;
        postFun.swap(m_postEventList);
        //LOG_STDOUT("Loop: Start run FuncHandler");
        for (size_t i = 0; i < postFun.size(); ++i){
            postFun[i](NULL);
        }
    }
}