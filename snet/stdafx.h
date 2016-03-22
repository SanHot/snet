//
//  stdafx.h
//  snet
//
//  Created by San on 15/8/18.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#ifndef __snet__stdafx_h
#define __snet__stdafx_h

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <assert.h>
#include <vector>
#include <string>
#include <map>
#include <list>

#include <time.h>
#include <memory>
#include <functional>
#include <mutex>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define random rand
#else
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

//typedef unsigned char uchar_t;
//#define interface class

#endif
