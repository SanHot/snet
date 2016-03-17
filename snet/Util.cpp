//
//  Util.cpp
//  snet
//
//  Created by San on 15/8/24.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include "Util.h"
#include <locale>

#define  MAX_PATH 260
#define MAX_LOG_FILE_SIZE	0x4000000

int Util::setSleep(int millisecond) {
#ifdef _WIN32
    Sleep(millisecond);
#else
    int sec = (int)(millisecond / 1000);
    millisecond = millisecond - (sec * 1000);
    struct timespec ts = { 0, millisecond * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
    return 0;
}

int Util::isFileExist(const char* path) {
    //00: exist, 02: read,  04: write, 06: read and write
    return access(path, 0);
}

std::wstring Util::utf8_to_wstring(const std::string &str) {
#if (_MSC_VER >= 1800)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
#else
    std::locale old_loc = std::locale::global(std::locale(""));
    const char* src_str = str.c_str();
    const size_t buffer_size = str.size() + 1;
    wchar_t dst_wstr[256] = { 0 };
#ifdef _MSC_VER
    size_t i;
    mbstowcs_s(&i, dst_wstr, 256, src_str, buffer_size);
#else
    mbstowcs(dst_wstr, src_str, buffer_size);
#endif
    std::locale::global(old_loc);
    
    return std::wstring(dst_wstr);
#endif
}

std::string Util::wstring_to_utf8(const std::wstring &str) {
#if (_MSC_VER >= 1800)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
#else
    std::locale old_loc = std::locale::global(std::locale(""));
    const wchar_t* src_wstr = str.c_str();
    size_t buffer_size = str.size() * 4 + 1;
    char dst_str[256] = { 0 };
#ifdef _MSC_VER
    size_t i;
    wcstombs_s(&i, dst_str, 256, src_wstr, buffer_size);
#else
    wcstombs(dst_str, src_wstr, buffer_size);
#endif
    std::locale::global(old_loc);
    
    return std::string(dst_str);
#endif
}

char* Util::GetIniKeyString(char *title, char *key, char *filename) {
    FILE *fp;
    char szLine[1024];
    static char tmpstr[1024];
    int rtnval;
    int i = 0;
    int flag = 0;
    char *tmp;
    
    if((fp = fopen(filename, "r")) == NULL)
        return NULL;
    
    while(!feof(fp)) {
        rtnval = fgetc(fp);
        if(rtnval == EOF)
            break;
        else
            szLine[i++] = rtnval;
        
        if(rtnval == '\n') {
#ifndef WIN32
            i--;
#endif
            szLine[--i] = '\0';
            i = 0;
            tmp = strchr(szLine, '=');
            
            if(( tmp != NULL )&&(flag == 1)) {
                if(strstr(szLine,key)!=NULL) {
                    //注释行
                    if ('#' == szLine[0] || ('/' == szLine[0] && '/' == szLine[1]) ) {
                    }
                    else {
                        //找打key对应变量
                        strcpy(tmpstr,tmp+1);
                        fclose(fp);
                        return tmpstr;
                    }
                }
            }
            else {
                strcpy(tmpstr,"[");
                strcat(tmpstr,title);
                strcat(tmpstr,"]");
                if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 ) {
                    //找到title
                    flag = 1;
                }
            }
        }
    }
    fclose(fp);
    return NULL;
}