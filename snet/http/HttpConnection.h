//
//  HttpConnection.hpp
//  snet
//
//  Created by San on 16/3/4.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef HttpConnection_h
#define HttpConnection_h

#include "../TcpStream.h"
#include "../Callback.h"
#include "../Buffer.h"
#include "HttpResponse.h"
#include "HttpParserObj.h"

#define READ_BUFF_SIZE 0x100000
#define UPLOAD_FILE_SIZE 0xA00000

class HttpResponse;
class HttpConnection;

typedef std::shared_ptr<HttpConnection> HttpConnectionPtr_t;

//std::map<int, HttpConnectionPtr_t> g_httpConnMap;
//
//HttpConnectionPtr_t findHttpConn(int fd) {
//    auto iter = g_httpConnMap.find(fd);
//    if (iter != g_httpConnMap.end())
//        return iter->second;
//    return NULL;
//}

class HttpConnection: public std::enable_shared_from_this<HttpConnection>
{
public:
    typedef std::function<void(uint8_t method, const std::string&, HttpResponse*)> HttpCallback_t;
    HttpConnection(StreamPtr_t conn): m_conn(conn) {}
    ~HttpConnection(){}
    
    void setCallback(HttpCallback_t callback){m_callback = callback;}
    
    void onRequest(const char* buf, int len) {
        if (m_input.size() - m_input.offset() < READ_BUFF_SIZE +1)
            m_input.extend(READ_BUFF_SIZE+1);
        m_input.write((void*)buf, len);
        
        HttpParserObj* obj = HttpParserObj::GetInstance();
        obj->parseBuffer(m_input);
        
        if(obj->ready()) {
            m_input.read(NULL, len);
            HttpResponse res;
            if (m_callback != NULL)
                m_callback(obj->method(), obj->url(), &res);
            else
                res.setHttp404Status();
            std::string data = res.packet();
            auto stream = m_conn.lock();
            stream->async_write(data.c_str(), (int)data.length(), NULL);
        }
    }
    void onClose() {
        
    }
    
private:
    void onWriteComplete() {
        
    }
    
private:
    std::weak_ptr<TcpStream> m_conn;
    HttpCallback_t m_callback;
    Buffer m_input;
    Buffer m_output;
    time_t m_last_read_time;
};

#endif /* HttpConnection_h */
