//
//  Util.h
//  snet
//
//  Created by San on 15/8/24.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#ifndef __snet__Util__
#define __snet__Util__

#include "stdafx.h"

#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
TypeName(const TypeName&);                         \
void operator=(const TypeName&)

struct Util
{
    static int setSleep(int millisecond);
    static int isFileExist(const char* path);
    static char *GetIniKeyString(char *title,char *key,char *filename);
    static std::wstring utf8_to_wstring(const std::string& str);
    static std::string wstring_to_utf8(const std::wstring& str);
};

#endif /* defined(__snet__Util__) */
