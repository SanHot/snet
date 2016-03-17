//
//  BaseSock.h
//  snet
//
//  Created by San on 15/8/17.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#ifndef __snet__BaseSock__
#define __snet__BaseSock__

#include "stdafx.h"

struct BaseSocket
{
    static int init_fd();
    static int close_fd(int fd);
    static int start_listen(int fd, const char* ip, int port);
    static int connect(int fd, const char* ip, int port);
    static int get_arrivals(int fd);
    static int set_reuse_addr(int fd, bool on);
    static int set_non_block(int fd);
    static int set_no_delay(int fd);
    static int is_blocking(int error_code);
    static int get_error_code(int fd);
};



#endif /* defined(__snet__BaseSock__) */
