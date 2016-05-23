//
//  IOStream.cpp
//  snet
//
//  Created by San on 15/8/19.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include "TcpStream.h"
#include "IOLoop.h"
#include "IOEvent.h"
#include "Log.h"

StreamMap_t g_streamMap;

StreamPtr_t FindConnectedStream(int fd) {
    auto iter = g_streamMap.find(fd);
    if (iter != g_streamMap.end()) {
        return iter->second;
    }
    return NULL;
}

TcpStream::TcpStream(IOLoop* loop, int fd)
:m_loop(loop)
,m_fd(fd)
,m_ev(NULL)
,m_status(STATE_UNINIT)
,m_write_buffer_size(1048576)
,m_read_buffer_size(1048576)
{
    if(m_fd == -1) {
        m_fd = BaseSocket::init_fd();
        m_status = STATE_UNINIT;
    }
    else {
        m_status = STATE_CONNECTED;
    }
    m_ev = new IOEvent(m_loop, m_fd);
    m_ev->setReadCallback(std::bind(&TcpStream::handle_read_event, this, std::placeholders::_1));
    m_ev->setWriteCallback(std::bind(&TcpStream::handle_write_event, this, std::placeholders::_1));
//    m_ev->setErrorCallback(std::bind(&TcpStream::handle_error_event, this, std::placeholders::_1));
}

TcpStream::~TcpStream()
{
    delete m_ev;
}

//read事件中有2种情况
//1， （针对服务器）accept队列中有新的client，
//          开始对client的connect连接，标示readable推入新事件
//2， （针对所有）已经完成连接的peer有新数据写入
//          该情况分2种，请检查新数据的fionread，
//          如果为0，表示client完成close，
//          如果不为0， 有新数据正常到达。
void TcpStream::handle_read_event(void* arg) {
    if (m_status == STATE_LISTENING) {
//        情况1: 开始accepte新连接
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(sockaddr_in);

        while (true) {
            int peer_fd = accept(m_fd, (sockaddr*)&peer_addr, &addr_len);
            if (peer_fd == SOCKET_ERROR)
                break;
            
            BaseSocket::set_no_delay(peer_fd);
            BaseSocket::set_non_block(peer_fd);
            
            StreamPtr_t peer_stream = std::make_shared<TcpStream>(m_loop, peer_fd);
            g_streamMap.insert(std::make_pair(peer_fd, peer_stream));

            peer_stream->m_read_callback = m_read_callback;
            peer_stream->event()->setReading(true);
            
            peer_stream->m_peer_addr = std::move(IPAddress(peer_addr, peer_fd));
            peer_stream->m_local_addr = m_local_addr;
            
            LOG_STDOUT("Accepted(%d): %s,%d", peer_fd, peer_stream->m_peer_addr.ipStr().c_str(), peer_stream->m_peer_addr.port());
            
            if (m_accept_callback != NULL)
                m_accept_callback(peer_stream);
        }
    }
    else {
//        情况2: 开始read数据
//        if (m_status == STATE_CLOSING)
//            return;
        assert(m_status == STATE_CONNECTED);
        uint32_t free = m_readBuffer.size() - m_readBuffer.offset();
        if (free < m_read_buffer_size + 1)
            m_readBuffer.extend(m_read_buffer_size + 1);
        ssize_t ret = ::recv(m_fd, m_readBuffer.buffer()+m_readBuffer.offset(), m_read_buffer_size, 0);
        if (ret > 0) {
            LOG_STDOUT("Recv(%d): %d", m_fd, (int)ret);
            m_readBuffer.incWriteOffset((int)ret);
            if (m_read_callback != NULL)
                m_read_callback(shared_from_this(), &m_readBuffer);
        }
        else if (ret == 0){
            post_close_event(NULL);
        }
        else {
            int err = BaseSocket::get_error_code();
            if (err != EAGAIN && err != EWOULDBLOCK) {
                LOG_STDOUT("READ_EVENT(%d): error(%d)", (int)ret, err);
                handle_error_event((void*)&err);
            }
        }
    }
}

//write事件中有2种情况
//1， （针对客户端）在开始异步connect后读到writable事件后表示完成异步connect，
//        这时候需要检查connect情况，确保连接正常
//2， （针对所有）已经完成连接的peer有可以开始发送数据，
//         在这个事件可使用send发送数据
void TcpStream::handle_write_event(void* arg) {
    if (m_status == STATE_CONNECTING) {
        int err = 0;
#ifdef _WIN32
        int len = sizeof(err);
        getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
#else
        unsigned int len = sizeof(err);
        getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (void*)&err, &len);
#endif
        if (err) {
            LOG_STDOUT("CONNECTING_EVENT(%d): error(%d)", m_fd, err);
            handle_error_event((void*)&err);
        } else {
            m_status = STATE_CONNECTED;
            if (m_connect_callback != NULL)
                m_connect_callback(shared_from_this());
        }
        //    完成一次write事件
        m_ev->setWriting(false);
    } else {
        if (m_status == STATE_CLOSING)
            return;
        assert(m_status == STATE_CONNECTED);
        if (m_ev->isWriting()) {
            if (m_sendBuffer.size() == 0) {
                m_ev->setWriting(false);
                return;
            }

#ifdef _WIN32
            ssize_t ret = ::send(m_fd, m_sendBuffer.buffer(), m_sendBuffer.offset(), 0);
#else
            struct iovec vector;
            vector.iov_base = m_sendBuffer.buffer();
            vector.iov_len = m_sendBuffer.offset();
            ssize_t ret = ::writev(m_fd, &vector, 1);
#endif
            LOG_STDOUT("Write(%d): %d", m_fd, (int)ret);
            if (ret == m_sendBuffer.offset()) {
                m_sendBuffer.clear();
                m_ev->setWriting(false);
                if (m_write_callback != NULL)
                    m_write_callback(shared_from_this());
                
            }
            else if (ret < 0) {
                int err = BaseSocket::get_error_code();
                if (err != EAGAIN && err != EWOULDBLOCK) {
                    m_sendBuffer.clear();
                    m_ev->setWriting(false);
                    LOG_STDOUT("WRITE_EVENT(%d): error(%d)", m_fd, err);
                    handle_error_event((void*)&err);
                }
            }
            else {
                m_sendBuffer.read(NULL, (uint32_t)ret);
            }
        }
    }
}

void TcpStream::post_close_event(void* arg) {
    m_status = STATE_CLOSING;
    m_loop->add_handle(std::bind(&TcpStream::handle_close_event, this, std::placeholders::_1));
}

void TcpStream::handle_close_event(void* arg) {
    assert(m_status == STATE_CLOSING);
    m_loop->remove_timer(m_timer);
    
    IPAddress addr(m_peer_addr);
    int svr_fd = m_local_addr.fd();
    int lcl_fd = m_fd;
    
    m_ev->setClosing();
    size_t n = g_streamMap.erase(lcl_fd);
    assert(n == 1);
#ifdef _WIN32
    ::closesocket(lcl_fd);
#else
    ::close(lcl_fd);
#endif
    LOG_STDOUT("Closed(%d): %s,%d", addr.fd(), addr.ipStr().c_str(), addr.port());
    
    StreamPtr_t svr = FindConnectedStream(svr_fd);
    if (svr != NULL && svr->m_close_callback != NULL)
        svr->m_close_callback(addr);
}

void TcpStream::handle_error_event(void* arg) {
    if (m_error_callback != NULL)
        m_error_callback(m_peer_addr, arg);
}

void TcpStream::handle_timeout_event(void* arg) {
    uint64_t now = *((uint64_t *)arg);
    if (m_timeout_callback) {
        m_timeout_callback(shared_from_this(), now);
    }
}

int TcpStream::async_listening(const char* ip, int port, const ConnectionCallback_t& callback) {
    if (m_fd == -1)
        return -1;
    
    int ret = BaseSocket::start_listen(m_fd, ip, port);
    if (ret == -1) {
        BaseSocket::close_fd(m_fd);
        return -1;
    }
    
    m_local_addr = std::move(IPAddress(ip, port, m_fd));
    m_accept_callback = callback;
    m_status = STATE_LISTENING;
    g_streamMap.insert(std::make_pair(m_fd, shared_from_this()));
    m_ev->setReading(true);
    
    LOG_STDOUT("Listening: %s,%d", ip, port);
    return ret;
}

int TcpStream::async_connect(const char* server_ip, int server_port, const ConnectionCallback_t& callback) {
    if (m_fd == -1)
        return -1;
    
    int ret = BaseSocket::connect(m_fd, server_ip, server_port);
    if (ret == -1) {
        BaseSocket::close_fd(m_fd);
        return -1;
    }
    
    m_connect_callback = callback;
    m_status = STATE_CONNECTING;
    g_streamMap.insert(std::make_pair(m_fd, shared_from_this()));
    m_ev->setWriting(true);
    
    LOG_STDOUT("Connecting: %s,%d", server_ip, server_port);
    return ret;
}

int TcpStream::async_close() {
    post_close_event(NULL);
    return 0;
}

int TcpStream::async_write(const char* data, int len) {
    m_sendBuffer.write((void*)data, len);
    m_ev->setWriting(true);
    return 0;
}

int TcpStream::async_write(const char* data, int len, const WriteCompleteCallback_t& callback) {
    m_sendBuffer.write((void*)data, len);
    m_write_callback = callback;
    m_ev->setWriting(true);
    return 0;
}

int TcpStream::async_read(const ReadMessageCallback_t& callback) {
    m_read_callback = callback;
    return 0;
}

int TcpStream::async_read_bytes(int num_of_bytes, const ReadMessageCallback_t& callback) {
    m_read_callback = callback;
    return 0;
}

int TcpStream::async_read_until(char delimiter, const ReadMessageCallback_t& callback) {
    m_read_callback = callback;
    return 0;
}

void TcpStream::setTimeOutCallback(uint32_t timeout, const TimerCallback_t &callback) {
    m_timeout_callback = callback;
    if (m_status != STATE_CONNECTING) {
        m_timer = m_loop->add_timer(timeout, std::bind(&TcpStream::handle_timeout_event, this, std::placeholders::_1));
    }
}
