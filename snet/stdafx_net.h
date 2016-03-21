//
//  stdafx_net.h
//  snet
//
//  Created by San on 16/2/29.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__stdafx_net_h
#define __snet__stdafx_net_h

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int	socklen_t;
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#define SOCKET_ERROR -1
#endif
#include <errno.h>
#define INVALID_SOCKET -1

#endif /* __snet__stdafx_net_h */
