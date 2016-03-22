//
//  BaseSock.cpp
//  snet
//
//  Created by San on 15/8/17.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include "BaseSocket.h"
#include "Log.h"
#include "stdafx_net.h"

int BaseSocket::START_UP() {
#ifdef _WIN32
    WSADATA wsaData;
    WORD wReqest = MAKEWORD(1, 1);
    if (WSAStartup(wReqest, &wsaData) != 0)
        return -1;
#endif
    return 0;
}

int BaseSocket::CLEAN_UP() {
#ifdef _WIN32
    if (WSACleanup() != 0)
        return -1;
#endif
    return 0;
}

int BaseSocket::init_fd(){
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return fd;
}
int BaseSocket::close_fd(int fd) {
    return close(fd);
}

int BaseSocket::start_listen(int fd, const char* ip, int port) {
    int err = 0;
    err = set_reuse_addr(fd, true);
    err = set_non_block(fd);
    
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = inet_addr(ip);
    if (local_addr.sin_addr.s_addr == INADDR_NONE) {
        hostent* host = gethostbyname(ip);
        if (host == NULL) {
            LOG_STDOUT("gethostbyname(%d): error(%s:%d)", fd, ip, port);
            return -1;
        }
        local_addr.sin_addr.s_addr = *(uint32_t*)host->h_addr_list[0];
    }
    
    err = bind(fd, (sockaddr*)&local_addr, sizeof(local_addr));
    if (err < 0) {
        LOG_STDOUT("bind(%d): error(%s:%d)", fd, ip, port);
        return -1;
    }
    
    err = listen(fd, 64);
    if (err < 0) {
        LOG_STDOUT("listen(%d): error(%s:%d)", fd, ip, port);
        return -1;
    }
    
    return 0;
}

int BaseSocket::connect(int fd, const char* ip, int port) {
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(sockaddr_in));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(ip);
    
    set_non_block(fd);
    int err = ::connect(fd, (sockaddr*)&remote_addr, sizeof(remote_addr));
    if(err < 0) {
        LOG_STDOUT("connect(%d): error(%s:%d)", fd, ip, port);
        return -1;
    }
    
    return 0;
}

int BaseSocket::get_arrivals(int fd) {
    u_long reach_count = 0;
#ifdef _WIN32
    int ret = ioctlsocket(fd, FIONREAD, &reach_count);
#else
    int ret = ioctl(fd, FIONREAD, &reach_count);
#endif
    if (ret < 0) {
        LOG_STDOUT("ioctl(%d): error", fd);
        return -1;
    }
    
    return (int)reach_count;
}

int BaseSocket::set_reuse_addr(int fd, bool on) {
    int opt = on ? 1 : 0;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, static_cast<socklen_t>(sizeof(opt)));
}

int BaseSocket::set_non_block(int fd) {
#ifdef _WIN32
    u_long nonblock = 1;
    return ioctlsocket(fd, FIONBIO, &nonblock);
#else
    return fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
}

int BaseSocket::set_no_delay(int fd) {
    int nodelay = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
}

int BaseSocket::is_blocking(int error_code) {
    
    return 0;
}

int BaseSocket::get_error_code() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}
