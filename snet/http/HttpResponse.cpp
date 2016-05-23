//
//  HttpResponse.cpp
//  snet
//
//  Created by San on 16/3/22.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "HttpResponse.h"

void HttpResponse::setStatusCode(Http_Code code){
    m_code = code;
    switch (code) {
        case HTTP_OK:          m_des = "OK";break;
        case HTTP_REQERROR:    m_des = "BAD REQUEST";break;
        case HTTP_NOTFAND:     m_des = "NOT FOUND";break;
        case HTTP_METHODERROR: m_des = "Method Not FOUND";break;
        default:break;
    }
}

void HttpResponse::setHttp404Status(const char* error_msg) {
    setStatusCode(HTTP_NOTFAND);
    setContentType(CONTEXT_TYPE_HTML);
    addHeader("Server", JOINTCOM_FLAG);
    if (error_msg)
        setBody(error_msg);
    else
        setBody("<HTML><TITLE>Not Found</TITLE>\r\n"
            "<BODY><P>(CODE: 404)</P>\r\n"
            "the resource is unavailable or nonexistent.\r\n"
            "</BODY></HTML>\r\n");
}

void HttpResponse::setHttp400Status() {
    setStatusCode(HTTP_REQERROR);
    setContentType(CONTEXT_TYPE_HTML);
    addHeader("Server", JOINTCOM_FLAG);
    setBody("<HTML><TITLE>Not Found</TITLE>\r\n"
            "<BODY><P>(CODE: 400)</P>\r\n"
            "Your browser sent a bad request.\r\n"
            "</BODY></HTML>\r\n");
}

void HttpResponse::setHttp501Status() {
    setStatusCode(HTTP_METHODERROR);
    setContentType(CONTEXT_TYPE_HTML);
    addHeader("Server", JOINTCOM_FLAG);
    setBody("<HTML><TITLE>Method Not FOUND</TITLE>\r\n"
            "<BODY><P>(CODE: 501)</P>\r\n"
            "HTTP request method not supported.\r\n"
            "</BODY></HTML>\r\n");
}

std::string HttpResponse::packet(){
    std::string ret;
    char buf[32] = {0};
    
    snprintf(buf, sizeof buf, "HTTP/1.1 %d %s\r\n", m_code, m_des.c_str());
    ret+=(buf);
    
    if (!m_alive)
        ret+=("Connection: close\r\n");
    else
        ret+=("Connection: Keep-Alive\r\n");
    
    snprintf(buf, sizeof buf, "Content-Length: %d\r\n", (int)m_body.size());
    ret+=(buf);
    
    for (auto it = m_headers.begin(); it != m_headers.end(); ++it) {
        ret+=(it->first);
        ret+=(": ");
        ret+=(it->second);
        ret+=("\r\n");
    }
    ret+=("\r\n");
    ret+=(m_body);

    //LOG_STDOUT("Response: %s", ret.c_str());
    return ret;
}
