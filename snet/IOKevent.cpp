//
//  IOLoop.cpp
//  snet
//
//  Created by San on 15/8/19.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#ifdef __APPLE__
#include <sys/event.h>
#include "IOKevent.h"
#include "IOEvent.h"
#include "Log.h"
#define MIN_TIMER_DURATION 100 //ms

IOKevent::IOKevent()
:m_running(false)
{
    m_kqfd = kqueue();
    if (m_kqfd == -1) {
        LOG_STDOUT("kqueue: failed");
    }
}

IOKevent::~IOKevent() {}

int IOKevent::processEvent(EventList* activeList) {
    struct kevent kevents[1024];
    struct timespec timeout;
    timeout.tv_sec = 0; //秒
    timeout.tv_nsec = MIN_TIMER_DURATION * 1000000;//那秒
    
    int nfds = kevent(m_kqfd, NULL, 0, kevents, 1024, &timeout);
    //int err = (-1 == nfds) ? errno : 0;

    for (int i = 0; i < nfds; i++)
    {//响应事件
        uintptr_t ev_fd = kevents[i].ident;
        auto it = m_event_map.find((int)ev_fd);
        IOEvent* io = it->second;
        io->poll_events = EVENT_NONE;
        
        if (kevents[i].filter == EVFILT_READ) {
            //LOG_STDOUT("EVFILT_READ: socket=(%d)", (int)ev_fd);
            io->poll_events |= EVENT_READ;
        }
        if (kevents[i].filter == EVFILT_WRITE) {
            //LOG_STDOUT("EVFILT_WRITE: socket=(%d)", (int)ev_fd);
            io->poll_events |= EVENT_WRITE;
        }
        activeList->push_back(io);
    }
    
    return 0;
}

int IOKevent::add_handle(IOEvent* ev) {
    struct kevent ke;
    
    if ((ev->events & EVENT_READ) != 0) {
        EV_SET(&ke, ev->fd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }
    
    if ((ev->events & EVENT_WRITE) != 0) {
        EV_SET(&ke, ev->fd(), EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }
    
    m_event_map.insert(std::make_pair(ev->fd(), ev));
    return 0;
}

int IOKevent::update_handle(IOEvent* ev, int next_events) {
    struct kevent ke;
    int fd = ev->fd();
    if(m_event_map.find(ev->fd()) == m_event_map.end()) {
        ev->events = next_events;
        add_handle(ev);
    }
    else {
        if ((next_events & EVENT_READ) != 0) {
            if ((ev->events & EVENT_READ) == 0) {
                EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
            }
        }
        else {
            EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
        }
    
        if ((next_events & EVENT_WRITE) != 0) {
            if ((ev->events & EVENT_WRITE) == 0) {
                EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
            }
        }
        else {
            EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
        }
        ev->events = next_events;
    }
    
    return 0;
}

int IOKevent::remove_handle(IOEvent* ev) {
    int fd = ev->fd();
    int events = m_event_map[fd]->events;
    struct kevent ke;
    
    if ((events & EVENT_READ) != 0) {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }
    
    if ((events & EVENT_WRITE) != 0) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }
    
    m_event_map.erase(fd);
    return 0;
}

bool IOKevent::isPollReading(int fd) {
    return false;
}

bool IOKevent::isPollWriting(int fd) {
    return false;
}

#endif