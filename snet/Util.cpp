//
//  Util.cpp
//  snet
//
//  Created by San on 15/8/24.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include "Util.h"
#include <locale>
#include <sys/time.h>

#define  MAX_PATH 260
#define MAX_LOG_FILE_SIZE	0x4000000

int setSleep(int millisecond) {
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

uint64_t get_tick() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return tval.tv_sec * 1000L + tval.tv_usec / 1000L;
}

int get_now(char* date_time) {
    time_t now = time(NULL);
    char date[128] = { 0 };
    struct tm *p = localtime(&now);
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
              p->tm_hour, p->tm_min, p->tm_sec);
    memcpy(date_time, date, strlen(date));
    return 0;
}

int isFileExist(const char* path) {
    //00: exist, 02: read,  04: write, 06: read and write
    return access(path, 0);
}

char* GetIniKeyString(char *title, char *key, char *filename) {
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
#ifndef _WIN32
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