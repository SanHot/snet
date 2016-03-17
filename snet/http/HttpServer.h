//
//  HttpServer.hpp
//  snet
//
//  Created by San on 16/3/2.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef HttpServer_hpp
#define HttpServer_hpp

#include "../stdafx.h"
#include "../IOStream.h"
#include "../IOLoop.h"
#include "../Callback.h"
#include "../Log.h"
#include "../MutexGuard.h"
#include "HttpResponse.h"
#include "HttpParserObj.h"
#include "TreadPool.h"

typedef std::function<void(const std::string&, HttpResponse*)> HttpCallback_t;
typedef std::map<int, HttpResponse>  SendList_t;
class HttpTask: public Task
{
public:
    HttpTask(int fd, std::string url):m_fd(fd),m_url(url)
    {}
    virtual ~HttpTask() {}
    void setHttpCallback(HttpCallback_t callback) {m_callback = callback;}
    void run();
private:
    int m_fd;
    int m_method;
    std::string m_url;
    HttpCallback_t m_callback;
};

class HttpServer
{
public:
    HttpServer(IOLoop* loop);
    ~HttpServer();
    
    int start(const char* ip, int port);
    void setHttpCallback(const HttpCallback_t& callback);
    void addLoopFunc(IOLoop::Function_t func);
    static void addResponseList(int fd, const HttpResponse& res);
    static void sendResponseList();
    
private:
    void runInLoop(void* arg);
    void onConn(const StreamPtr_t& stream);
    void onRead(const StreamPtr_t& stream , Buffer* buf);
    void onWriten(const StreamPtr_t& stream);
    
private:
    IOLoop* m_loop;
    StreamPtr_t m_svr;
    HttpCallback_t m_httpCallback;
    
    ThreadPool g_httpThreadPool;
    static MUTEX_T s_mtx;
    static SendList_t s_sendList;
};

#endif /* HttpServer_hpp */
