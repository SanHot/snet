//
//  IOEvent.h
//  snet
//
//  Created by San on 16/2/17.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__IOEvent__
#define __snet__IOEvent__

#include "stdafx.h"

enum {
    EVENT_NONE      = 0x0,
    EVENT_READ		= 0x1,
    EVENT_WRITE	    = 0x2,
    EVENT_ERROR	    = 0x4,
    EVENT_ALL		= 0x7
};

class IOLoop;
class IOEvent
{
public:
    typedef std::function<void (void*)> Callback_t;
    
    IOEvent(IOLoop* loop, int fd);
    ~IOEvent(){}
    int fd() const {return m_fd;}
    IOLoop* looper() {return m_loop;}
    void setReading(bool isReading);
    void setWriting(bool isWriting);
    void setClosing();
    bool isReading() const {return (EVENT_READ & events);}
    bool isWriting() const {return (EVENT_WRITE & events);}
    
    void handleEvent(void* arg);
    void setReadCallback(Callback_t cb) {m_read_callback = cb;}
    void setWriteCallback(Callback_t cb) {m_write_callback = cb;}
    void setErrorCallback(Callback_t cb) {m_error_callback = cb;}
    void setCloseCallback(Callback_t cb) {m_close_callback = cb;}
    
    int events;
    int poll_events;
    std::weak_ptr<void> stream;
    
private:
    IOLoop* m_loop;
    int m_fd;
    Callback_t m_read_callback;
    Callback_t m_write_callback;
    Callback_t m_close_callback;
    Callback_t m_error_callback;
};

#endif /* __snet__IOEvent__ */
