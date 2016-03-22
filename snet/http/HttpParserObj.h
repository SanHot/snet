//
//  HttpParserObj.h
//  snet
//
//  Created by San on 16/3/5.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef HttpParserObj_h
#define HttpParserObj_h

#include "../stdafx.h"
#include "http_parser.h"

class Buffer;
class HttpParserObj {
public:
    virtual ~HttpParserObj() {}
    
    static HttpParserObj* GetInstance(){
        if (!m_instance) {
            m_instance = new HttpParserObj();
        }
        return m_instance;
    }
    
    void parseBuffer(Buffer buf);
    void parseBuffer(const char* buf, uint32_t len);
    
    bool ready() {return m_read_all;}
    uint32_t length() {return m_total_length;}
    uint8_t method() {return m_method;}
    std::string& url() {return m_url;}
    std::string& body() {return m_body_content;}
    
    void setUrl(const char* url, size_t length) {m_url.append(url, length);}
    void setBodyContent(const char* content, size_t length) {m_body_content.append(content, length);}
    void setTotalLength(uint32_t total_len) {m_total_length = total_len;}
    void setReady() {m_read_all = true;}
    
    static int onUrl(http_parser* parser, const char *at, size_t length){
        m_instance->setUrl(at, length);
        return 0;
    }
    static int onHeaderField(http_parser* parser, const char* at, size_t length){
        m_instance->setCurName(at, length);
        return 0;
    }
    static int onHeaderValue(struct http_parser* parser, const char* at, size_t length){
        m_instance->setCurValue(at, length);
        m_instance->addHeaders();
        return 0;
    }
    static int onHeadersComplete (http_parser* parser){
        m_instance->setTotalLength(parser->nread + (uint32_t)parser->content_length);
        m_instance->setMethod((http_method)parser->method);
        return 0;
    }
    static int onBody (http_parser* parser, const char *at, size_t length){
        m_instance->setBodyContent(at, length);
        return 0;
    }
    static int onMessageComplete (http_parser* parser){
        m_instance->setReady();
        return 0;
    }
private:
    //DISALLOW_EVIL_CONSTRUCTORS(HttpParserObj);
    HttpParserObj(){}
    void setMethod(http_method method) {
        switch (method) {
            case HTTP_GET: m_method = 1;break;
            case HTTP_DELETE: m_method = 2;break;
            case HTTP_POST: m_method = 3;break;
            case HTTP_PUT: m_method = 4;break;
            case HTTP_HEAD: m_method = 5;break;
            default: m_method = 0;break;
        }
    }
    void setCurName(const char* at, size_t length) {m_curHeaderName.assign(at,length);}
    void setCurValue(const char* at, size_t length) {m_curHeaderValue.assign(at,length);}
    void addHeaders(){m_headers.insert(std::make_pair(m_curHeaderName, m_curHeaderValue));}
    
private:
    
    static HttpParserObj*   m_instance;
    http_parser             m_http_parser;
    http_parser_settings    m_settings;
    bool        m_read_all;
    uint32_t    m_total_length;
    uint8_t m_method;
    std::string m_url;
    std::string m_body_content;
    std::string m_curHeaderName;
    std::string m_curHeaderValue;
    std::map<std::string, std::string> m_headers;
};

#endif /* HttpParserObj_h */
