//
//  HttpResponse.hpp
//  snet
//
//  Created by San on 16/3/2.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef HttpResponse_h
#define HttpResponse_h

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
    
    void setStatusCode(Http_Code code){
        m_code = code;
        switch (code) {
            case HTTP_OK:          m_des = "OK";break;
            case HTTP_REQERROR:    m_des = "BAD REQUEST";break;
            case HTTP_NOTFAND:     m_des = "NOT FOUND";break;
            case HTTP_METHODERROR: m_des = "Method Not FOUND";break;
            default:break;
        }
    }
    
    void setHttp404Status() {
        setStatusCode(HTTP_NOTFAND);
        setContentType("text/html");
        addHeader("Server", "Jointcom/snet1.0");
        setBody("<HTML><TITLE>Not Found</TITLE>\r\n"
                "<BODY><P>(CODE: 404)</P>\r\n"
                "the resource is unavailable or nonexistent.\r\n"
                "</BODY></HTML>\r\n");
    }
    
    void setHttp400Status() {
        setStatusCode(HTTP_REQERROR);
        setContentType("text/html");
        addHeader("Server", "Jointcom/snet1.0");
        setBody("<HTML><TITLE>Not Found</TITLE>\r\n"
                "<BODY><P>(CODE: 400)</P>\r\n"
                "Your browser sent a bad request.\r\n"
                "</BODY></HTML>\r\n");
    }
    
    void setHttp501Status() {
        setStatusCode(HTTP_METHODERROR);
        setContentType("text/html");
        addHeader("Server", "Jointcom/snet1.0");
        setBody("<HTML><TITLE>Method Not FOUND</TITLE>\r\n"
                "<BODY><P>(CODE: 501)</P>\r\n"
                "HTTP request method not supported.\r\n"
                "</BODY></HTML>\r\n");
    }
    
    std::string packet(){
        std::string ret;
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "HTTP/1.1 %d ", m_code);
        ret+=(buf);
        ret+=(m_des);
        ret+=("\r\n");
        if (!m_alive){
            ret+=("Connection: close\r\n");
        }
        else{
            snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", m_body.size());
            ret+=(buf);
            ret+=("Connection: Keep-Alive\r\n");
        }
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
    
private:
    std::map<std::string, std::string> m_headers;
    Http_Code m_code;
    std::string m_des;
    bool m_alive;
    std::string m_body;
};

#endif /* HttpResponse_h */
