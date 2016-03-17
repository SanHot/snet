//
//  Log.cpp
//  snet
//
//  Created by San on 16/2/29.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "Log.h"
#include <set>

Logger Logger::s_logger;
Logger::Logger() {}
Logger::~Logger() {
    for (auto& pr : m_fpmap) {
        if (pr.second) {
            fclose(pr.second);
        }
    }
}

int Logger::setPath(LogLevel level, const std::string& path) {
    auto it = s_logger.m_fpmap.find(level);
    if ( it == s_logger.m_fpmap.end()) {
        FILE* fp = fopen(path.c_str(), "a+");
        if (fp != nullptr)
            s_logger.m_fpmap.insert(std::make_pair(level, fp));
        return 0;
    }
    
    return -1;
}

FILE* Logger::fp(LogLevel level) {
    auto it = s_logger.m_fpmap.find(level);
    if (it != s_logger.m_fpmap.end()) {
        return it->second;
    }
    
    return stderr;
}