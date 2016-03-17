//
//  IOEvent.cpp
//  snet
//
//  Created by San on 16/2/25.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "IOEvent.h"
#include "IOLoop.h"

IOEvent::IOEvent(IOLoop* loop, int fd)
:m_loop(loop)
,m_fd(fd)
,events(EVENT_NONE)
,poll_events(EVENT_NONE)
,m_close_callback(NULL)
,m_error_callback(NULL)
,m_read_callback(NULL)
,m_write_callback(NULL)
{}

void IOEvent::setReading(bool isReading) {
    int next_events = events;
    if (isReading)
        next_events |= EVENT_READ;
    else
        next_events &= ~EVENT_READ;
    m_loop->update_handle(this, next_events);
}
void IOEvent::setWriting(bool isWriting) {
    int next_events = events;
    if (isWriting)
        next_events |= EVENT_WRITE;
    else
        next_events &= ~EVENT_WRITE;
    m_loop->update_handle(this, next_events);
}
void IOEvent::setClosing() {
    events = EVENT_NONE;
    m_loop->remove_handle(this);
}

void IOEvent::handleEvent(void* arg) {
    if (events & EVENT_NONE) {
        return;
    }
    //if (poll_events == EVFILT_READ) {
    if (isReading()) {
        if (m_read_callback) {
            m_read_callback(arg);
        }
    }
    //if (poll_events == EVFILT_WRITE) {
    if (isWriting()) {
        if (m_write_callback) {
            m_write_callback(NULL);
        }
    }
}

