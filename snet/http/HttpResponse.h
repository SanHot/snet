//
//  HttpResponse.hpp
//  snet
//
//  Created by San on 16/3/2.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef HttpResponse_h
#define HttpResponse_h

#include "../stdafx.h"
#include "stdafx_http.h"

class HttpResponse
{
public:
    enum Http_Code
    {
        HTTP_OK = 200,
        HTTP_REQERROR = 400,
        HTTP_NOTFAND = 404,
        HTTP_METHODERROR = 501
    };
    
    HttpResponse(bool alive = true): m_alive(alive) {setStatusCode(HTTP_NOTFAND);}
    
    void setAlive(bool on){m_alive = on;}
    bool Alive() const{return m_alive;}
    void setContentType(const std::string& contentType){addHeader("Content-Type", contentType);}
    void addHeader(const std::string& key, const std::string& value){m_headers.insert(std::make_pair(key, value));}
    void setBody(const std::string& body){m_body = body;}
    void setBody(const char* body, size_t len) {m_body.assign(body, len);}
    
    void setStatusCode(Http_Code code);
    void setHttp404Status(const char* error_msg = nullptr);
    void setHttp400Status();
    void setHttp501Status();
    
    std::string packet();
    
private:
    std::map<std::string, std::string> m_headers;
    Http_Code m_code;
    std::string m_des;
    bool m_alive;
    std::string m_body;
};

#endif /* HttpResponse_h */
