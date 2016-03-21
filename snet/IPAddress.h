//
//  IPAddress.h
//  snet
//
//  Created by San on 16/2/29.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__IPAddress_h
#define __snet__IPAddress_h

#include "stdafx.h"
#include "stdafx_net.h"

class IPAddress
{
public:
    IPAddress():m_fd(-1) {}
    ~IPAddress() {}
    
    IPAddress(const std::string& ip, uint16_t port, int fd) {
#ifdef WIN32
        ZeroMemory(&m_addr,  sizeof(m_addr));
#else
        bzero(&m_addr, sizeof(m_addr));
#endif
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        
        if(inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) <= 0) {
            uint32_t ret = getHostByName(ip);
            if(ret != uint32_t(-1))
                m_addr.sin_addr.s_addr = ret;
            else
                m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        
        m_ipstr = ip;
        m_port = port;
        m_fd = fd;
    }
    
    IPAddress(const struct sockaddr_in& addr, int fd) {
        memcpy(&m_addr, &addr, sizeof(addr));
        char ip_str[64] = {0};
        uint32_t ip = ntohl(m_addr.sin_addr.s_addr);
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        m_ipstr = std::string(ip_str);
        m_port = port();
        m_fd = fd;
    }

    const struct sockaddr_in& sockAddr() const { return m_addr; }
    std::string ipStr() const {return m_ipstr;}
    uint32_t ip() const {return ntohl(m_addr.sin_addr.s_addr);}
    uint16_t port() const {return ntohs(m_addr.sin_port);}
    int fd() const {return m_fd;}
    
    static int getLocalAddr(int sockFd, IPAddress& addr) {
        struct sockaddr_in sockAddr;
        socklen_t sockLen = sizeof(sockAddr);
        if(getsockname(sockFd, (struct sockaddr*)&sockAddr, &sockLen) != 0) {
            int err = errno;
            return err;
        }
        addr = IPAddress(sockAddr, sockFd);
        return 0;
    }
    
    static int getRemoteAddr(int sockFd, IPAddress& addr) {
        struct sockaddr_in sockAddr;
        socklen_t sockLen = sizeof(sockAddr);
        if(getpeername(sockFd, (struct sockaddr*)&sockAddr, &sockLen) != 0) {
            int err = errno;
            return err;
        }
        addr = IPAddress(sockAddr, sockFd);
        return 0;
    }
    
private:
    uint32_t getHostByName(const std::string& host) {
        struct addrinfo hint;
        struct addrinfo *answer;
        
        memset(&hint, 0, sizeof(hint));
        hint.ai_family = AF_INET;
        hint.ai_socktype = SOCK_STREAM;
        
        int ret = getaddrinfo(host.c_str(), NULL, &hint, &answer);
        if(ret != 0) {
            //printf("getaddrinfo error %s", errorMsg(errno));
            return uint32_t(-1);
        }

        for(struct addrinfo* cur = answer; cur != NULL; cur = cur->ai_next) {
            return ((struct sockaddr_in*)(cur->ai_addr))->sin_addr.s_addr;
        }
        
        printf("getHostByName Error");
        return uint32_t(-1);
    }

#ifdef WIN32
    int inet_pton(int af, const char *src, void *dst) {
        struct sockaddr_storage ss;
        int size = sizeof(ss);
        char src_copy[INET6_ADDRSTRLEN+1];

        ZeroMemory(&ss, sizeof(ss));
        /* stupid non-const API */
        strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
        src_copy[INET6_ADDRSTRLEN] = 0;

        if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
            switch(af) {
                case AF_INET:
                    *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
                    return 1;
                case AF_INET6:
                    *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
                    return 1;
            }
        }
        return 0;
    }
#endif
    
private:
    struct sockaddr_in m_addr;
    std::string m_ipstr;
    int m_port;
    int m_fd;
};

#endif /* __snet__IPAddress_h */
