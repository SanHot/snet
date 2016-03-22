//
//  TcpServer.h
//  snet
//
//  Created by San on 16/3/2.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef TcpServer_h
#define TcpServer_h

#include "TcpStream.h"

class TcpServer {
    
public:
    TcpServer():m_loop(new IOLoop),m_svr(std::make_shared<TcpStream>(m_loop)) {}
    ~TcpServer() {}
    
    void setCloseCallback();
    void setErrorCallback();
    void setReadCallback();
    void setErrorCallback(const ErrorCallback_t& callback) {m_svr->setErrorCallback(callback);}
    void setCloseCallback(const CloseCallback_t& callback) {m_svr->setCloseCallback(callback);}
    void start() {}
private:
    IOLoop* m_loop;
    StreamPtr_t m_svr;
    SOCKET m_fd;
};

#endif /* TcpServer_h */
