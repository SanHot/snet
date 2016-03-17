//
//  Log.h
//  snet
//
//  Created by San on 16/2/29.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__Log_h
#define __snet__Log_h

#include "stdafx.h"
#include "MutexGuard.h"

class Logger
{
public:
    enum LogLevel {
        debug = 1,
        notice,
        warning,
        fatal,
        other
    };
    
public:
    static int setPath(LogLevel level = other, const std::string& path = "");
    static FILE* fp(LogLevel level = other);
    static MUTEX_T& fp_mutex() { return s_logger.m_mtx; }
    
private:
    Logger();
    ~Logger();
    
private:
    MUTEX_T m_mtx;
    std::map<LogLevel, FILE*> m_fpmap;
    
    static Logger s_logger;
};

#define LOG_FATAL(fmt, args...) \
if (Logger::fp(Logger::fatal)) {\
MUTEXGUARD_T lck(Logger::fp_mutex());\
time_t __timer__ = time(NULL);\
struct tm* __tm__ = localtime(&__timer__);\
fprintf(Logger::fp(Logger::fatal), "[%d-%d-%d %d:%d:%d] [%s:%d]", __tm__->tm_year + 1900, __tm__->tm_mon + 1, __tm__->tm_mday, __tm__->tm_hour, __tm__->tm_min, __tm__->tm_sec, __FILE__, __LINE__);\
fprintf(Logger::fp(Logger::fatal), fmt, ##args);\
fprintf(Logger::fp(Logger::fatal), "%c", '\n');\
}

#define LOG_WARNING(fmt, args...) \
if (Logger::fp(Logger::warning)) {\
MUTEXGUARD_T lck(Logger::fp_mutex());\
time_t __timer__ = time(NULL);\
struct tm* __tm__ = localtime(&__timer__);\
fprintf(Logger::fp(Logger::warning), "[%d-%d-%d %d:%d:%d] [%s:%d]", __tm__->tm_year + 1900, __tm__->tm_mon + 1, __tm__->tm_mday, __tm__->tm_hour, __tm__->tm_min, __tm__->tm_sec, __FILE__, __LINE__);\
fprintf(Logger::fp(Logger::warning), fmt, ##args);\
fprintf(Logger::fp(Logger::warning), "%c", '\n');\
}

#define LOG_NOTICE(fmt, args...) \
if (Logger::fp(Logger::notice)) {\
MUTEXGUARD_T lck(Logger::fp_mutex());\
time_t __timer__ = time(NULL);\
struct tm* __tm__ = localtime(&__timer__);\
fprintf(Logger::fp(Logger::notice), "[%d-%d-%d %d:%d:%d] [%s:%d]", __tm__->tm_year + 1900, __tm__->tm_mon + 1, __tm__->tm_mday, __tm__->tm_hour, __tm__->tm_min, __tm__->tm_sec, __FILE__, __LINE__);\
fprintf(Logger::fp(Logger::notice), fmt, ##args);\
fprintf(Logger::fp(Logger::notice), "%c", '\n');\
}

#define LOG_STDERR(fmt, args...) {\
MUTEXGUARD_T lck(Logger::fp_mutex());\
time_t __timer__ = time(NULL);\
struct tm* __tm__ = localtime(&__timer__);\
fprintf(stderr, "[%d-%d-%d %d:%d:%d] [%s:%d]", __tm__->tm_year + 1900, __tm__->tm_mon + 1, __tm__->tm_mday, __tm__->tm_hour, __tm__->tm_min, __tm__->tm_sec, __FILE__, __LINE__);\
fprintf(stderr, fmt, ##args);\
fprintf(stderr, "%c", '\n');\
}

#define LOG_STDOUT(fmt, args...) {\
MUTEXGUARD_T lck(Logger::fp_mutex());\
time_t __timer__ = time(NULL);\
struct tm* __tm__ = localtime(&__timer__);\
fprintf(stdout, "[%d-%d-%d %d:%d:%d]", __tm__->tm_year + 1900, __tm__->tm_mon + 1, __tm__->tm_mday, __tm__->tm_hour, __tm__->tm_min, __tm__->tm_sec);\
fprintf(stdout, fmt, ##args);\
fprintf(stdout, "%c", '\n');\
}

#endif /* __snet__Log_h */
