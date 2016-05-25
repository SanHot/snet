//
//  IOStream.h
//  snet
//
//  Created by San on 15/8/19.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#ifndef __snet__IOStream__
#define __snet__IOStream__

#include "stdafx.h"
#include "BaseSocket.h"
#include "IPAddress.h"
#include "Callback.h"
#include "Buffer.h"
#include "Util.h"

enum
{
    STATE_UNINIT,
    STATE_LISTENING,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_CLOSING,
    STATE_CLOSED
};

StreamPtr_t FindConnectedStream(int fd);
class TcpStream : public std::enable_shared_from_this<TcpStream>
{
public:
    TcpStream(IOLoop* loop, int fd = -1);
    virtual ~TcpStream();
    
public:
    IOEvent* event() {return m_ev;}
    IOLoop* looper() {return m_loop;}
    int status() const {return m_status;}
    int fd() const{return m_fd;}
    IPAddress getLocalAddress() const{return m_local_addr;}
    IPAddress getPeerAddress() const{return m_peer_addr;}
    
    virtual int async_listening(const char* ip, int port,
                                const ConnectionCallback_t& callback);
    virtual int async_connect(const char* server_ip, int server_port,
                              const ConnectionCallback_t& callback);
    virtual int async_close();
    virtual int async_write(const char* data, int len);
    virtual int async_write(const char* data, int len,
                            const WriteCompleteCallback_t& callback);
    virtual int async_read(const ReadMessageCallback_t& callback);
    virtual int async_read_until(char delimiter, const ReadMessageCallback_t& callback);
    virtual int async_read_bytes(int num_of_bytes, const ReadMessageCallback_t& callback);
    void setWritenCallback(const WriteCompleteCallback_t& callback) {m_write_callback = callback;}
    void setErrorCallback(const ErrorCallback_t& callback) {m_error_callback = callback;}
    void setCloseCallback(const CloseCallback_t& callback) {m_close_callback = callback;}
    void setTimeOutCallback(uint32_t timeout, const TimerCallback_t& callback);
    
protected:
    void handle_read_event(void* arg);
    void handle_write_event(void* arg);
    void post_close_event(void* arg);
    void handle_close_event(void* arg);
    void handle_error_event(void* arg);
    void handle_timeout_event(void* arg);
    
public:
    class OptionKey {
    public:
        bool operator<(const OptionKey& other) const {
            if (level == other.level) {
                return optname < other.optname;
            }
            return level < other.level;
        }
        int apply(int fd, int val) const {
            return setsockopt(fd, level, optname, (char*)&val, sizeof(val));
        }
        int level;
        int optname;
    };
    
    typedef std::map<OptionKey, int> OptionMap;
    static const OptionMap emptyOptionMap;
    
    template <typename T>
    int getSockOpt(int level, int optname, T* optval, socklen_t* optlen) {
        return getsockopt(m_fd, level, optname, (char*)optval, optlen);
    }
    
    template <typename T>
    int setSockOpt(int  level,  int  optname,  const T *optval) {
        return setsockopt(m_fd, level, optname, optval, sizeof(T));
    }

private:
    DISALLOW_EVIL_CONSTRUCTORS(TcpStream);
    int m_fd;
    int m_status;
    int m_error_code;
    IOLoop* m_loop;
    IOEvent* m_ev;
    
    IPAddress m_peer_addr;
    IPAddress m_local_addr;

    int m_read_buffer_size;
    int m_write_buffer_size;
    Buffer m_sendBuffer;
    Buffer m_readBuffer;
    
    char m_read_delimiter;
    int m_read_bytes;
    const TimeItem_t* m_timer;
    
    //事件回调
    ConnectionCallback_t m_accept_callback;
    ReadMessageCallback_t m_read_callback;
    WriteCompleteCallback_t m_write_callback;
    ConnectionCallback_t m_connect_callback;
    ErrorCallback_t m_error_callback;
    CloseCallback_t m_close_callback;
    TimerCallback_t m_timeout_callback;
};

#endif /* defined(__snet__IOStream__) */
