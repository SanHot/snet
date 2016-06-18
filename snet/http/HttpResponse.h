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
    HttpResponse(bool alive = true): m_alive(alive) {setStatusCode(Http_Code::HTTP_NOTFAND);}
    
    void setAlive(bool on){m_alive = on;}
    bool Alive() const{return m_alive;}
    void setContentType(const std::string& contentType){addHeader("Content-Type", contentType);}
    void addHeader(const std::string& key, const std::string& value){m_headers.insert(std::make_pair(key, value));}
    void setBody(const std::string& body){m_body = body;}
    void setBody(const char* body, size_t len) {m_body.assign(body, len);}
    
    void setStatusCode(int code);
    void setHttp404Status(const char* error_msg = nullptr);
    void setHttp400Status();
    void setHttp501Status();
    
    std::string dump();

private:
    class Reasons
    {
    public:
        Reasons() {
            m_reasons.resize(600);

            m_reasons[100] = "Continue";
            m_reasons[101] = "Switching Protocols";
            m_reasons[200] = "OK";
            m_reasons[201] = "Created";
            m_reasons[202] = "Accepted";
            m_reasons[203] = "Non-Authoritative Information";
            m_reasons[204] = "No Content";
            m_reasons[205] = "Reset Content";
            m_reasons[206] = "Partial Content";
            m_reasons[300] = "Multiple Choices";
            m_reasons[301] = "Moved Permanently";
            m_reasons[302] = "Found";
            m_reasons[303] = "See Other";
            m_reasons[304] = "Not Modified";
            m_reasons[305] = "Use Proxy";
            m_reasons[307] = "Temporary Redirect";
            m_reasons[400] = "Bad Request";
            m_reasons[401] = "Unauthorized";
            m_reasons[402] = "Payment Required";
            m_reasons[403] = "Forbidden";
            m_reasons[404] = "Not Found";
            m_reasons[405] = "Method Not Allowed";
            m_reasons[406] = "Not Acceptable";
            m_reasons[407] = "Proxy Authentication Required";
            m_reasons[408] = "Request Time-out";
            m_reasons[409] = "Conflict";
            m_reasons[410] = "Gone";
            m_reasons[411] = "Length Required";
            m_reasons[412] = "Precondition Failed";
            m_reasons[413] = "Request Entity Too Large";
            m_reasons[414] = "Request-URI Too Large";
            m_reasons[415] = "Unsupported Media Type";
            m_reasons[416] = "Requested range not satisfiable";
            m_reasons[417] = "Expectation Failed";
            m_reasons[500] = "Internal Server Error";
            m_reasons[501] = "Not Implemented";
            m_reasons[502] = "Bad Gateway";
            m_reasons[503] = "Service Unavailable";
            m_reasons[504] = "Gateway Time-out";
            m_reasons[505] = "HTTP Version not supported";
        }

        const std::string& getReason(int code) {
            static std::string unknown = "Unknown Error";
            if((size_t)code >= m_reasons.size()) {
                return unknown;
            }

            return m_reasons[code].empty() ? unknown : m_reasons[code];
        }

    private:
        std::vector<std::string> m_reasons;
    };
    
private:
    std::map<std::string, std::string> m_headers;
    int m_code;
    std::string m_des;
    bool m_alive;
    std::string m_body;
    Reasons m_reasons;
};

#endif /* HttpResponse_h */
