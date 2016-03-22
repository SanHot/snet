//
// Created by san on 16/3/22.
//

#ifdef _WIN32
#include "IOSelect.h"
#include "IOEvent.h"
#include "Log.h"
#define MIN_TIMER_DURATION 100 //ms

IOSelect::IOSelect()
:m_running(false)
{
    FD_ZERO(&m_read_set);
    FD_ZERO(&m_write_set);
    FD_ZERO(&m_error_set);
}

IOSelect::~IOSelect() {}

int IOSelect::processEvent(EventList* activeList) {
    struct timeval timeout;
    timeout.tv_sec = 0; //秒
    timeout.tv_usec = MIN_TIMER_DURATION * 1000;//u秒

    fd_set read_set, write_set, excep_set;
    {
        MUTEXGUARD_T(m_mtx);
        memcpy(&read_set, &m_read_set, sizeof(fd_set));
        memcpy(&write_set, &m_write_set, sizeof(fd_set));
        memcpy(&excep_set, &m_error_set, sizeof(fd_set));
    }

    int ret = select(0, &read_set, &write_set, &excep_set, &timeout);
    if (ret == SOCKET_ERROR)
        return ret;
    for (int i = 0; i < read_set.fd_count; i++)
    {
        SOCKET ev_fd = read_set.fd_array[i];
        auto it = m_event_map.find((int)ev_fd);
        IOEvent* io = it->second;
        io->poll_events = EVENT_READ;
        activeList->push_back(io);
    }
    for (int i = 0; i < write_set.fd_count; i++)
    {
        SOCKET ev_fd = write_set.fd_array[i];
        auto it = m_event_map.find((int)ev_fd);
        IOEvent* io = it->second;
        io->poll_events = EVENT_READ;
        activeList->push_back(io);
    }
    return 0;
}

int IOSelect::add_handle(IOEvent* ev) {
    MUTEXGUARD_T(m_mtx);
    int fd = ev->fd();
    if ((ev->events & EVENT_READ) != 0) {
        FD_SET(fd, &m_read_set);
    }
    if ((ev->events & EVENT_WRITE) != 0) {
        FD_SET(fd, &m_write_set);
    }
    if ((ev->events & EVENT_ERROR) != 0) {
        FD_SET(fd, &m_error_set);
    }

    m_event_map.insert(std::make_pair(ev->fd(), ev));
    return 0;
}

int IOSelect::update_handle(IOEvent* ev, int next_events) {
    int fd = ev->fd();
    if(m_event_map.find(ev->fd()) == m_event_map.end()) {
        ev->events = next_events;
        add_handle(ev);
    }
    else {
        MUTEXGUARD_T(m_mtx);
        if ((next_events & EVENT_READ) != 0) {
            if ((ev->events & EVENT_READ) == 0)
                FD_SET(fd, &m_read_set);
        }
        else {
            FD_CLR(fd, &m_read_set);
        }

        if ((next_events & EVENT_WRITE) != 0) {
            if ((ev->events & EVENT_WRITE) == 0)
                FD_SET(fd, &m_write_set);
        }
        else {
            FD_CLR(fd, &m_write_set);
        }
        ev->events = next_events;
    }

    return 0;
}

int IOSelect::remove_handle(IOEvent* ev) {
    MUTEXGUARD_T(m_mtx);
    int fd = ev->fd();
    int events = m_event_map[fd]->events;
    if ((events & EVENT_READ) != 0) {
        FD_CLR(fd, &m_read_set);
    }
    if ((events & EVENT_WRITE) != 0) {
        FD_CLR(fd, &m_write_set);
    }
    if ((events & EVENT_WRITE) != 0) {
        FD_CLR(fd, &m_error_set);
    }

    m_event_map.erase(fd);
    return 0;
}

bool IOSelect::isPollReading(int fd) {
    return false;
}

bool IOSelect::isPollWriting(int fd) {
    return false;
}

#endif