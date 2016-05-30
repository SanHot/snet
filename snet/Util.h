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

int setSleep(int millisecond);
uint64_t get_tick();
int get_now(char* date_time);
wchar_t* MBs2WCs(const char* pszSrc);
int isFileExist(const char* path);
char *GetIniKeyString(char *title,char *key,char *filename);

#endif /* defined(__snet__Util__) */
