//
//  stdafx_net.h
//  snet
//
//  Created by San on 16/2/29.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__stdafx_net_h
#define __snet__stdafx_net_h

#ifdef _WIN32
//typedef int	socklen_t;
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif
#include <errno.h>


#endif /* __snet__stdafx_net_h */
