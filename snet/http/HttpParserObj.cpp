//
//  HttpParserObj.cpp
//  snet
//
//  Created by San on 16/3/5.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "../Buffer.h"
#include "HttpParserObj.h"

HttpParserObj* HttpParserObj::m_instance = NULL;

void HttpParserObj::parseBuffer(Buffer buf) {
    parseBuffer(buf.buffer(), buf.offset());
}

void HttpParserObj::parseBuffer(const char* buf, uint32_t len){
    http_parser_init(&m_http_parser, HTTP_REQUEST);
    memset(&m_settings, 0, sizeof(m_settings));
    m_settings.on_url              = onUrl;
    m_settings.on_header_field     = onHeaderField;
    m_settings.on_header_value     = onHeaderValue;
    m_settings.on_headers_complete = onHeadersComplete;
    m_settings.on_body             = onBody;
    m_settings.on_message_complete = onMessageComplete;
    
    m_read_all = false;
    m_total_length = 0;
    m_url.clear();
    m_body_content.clear();
    
    http_parser_execute(&m_http_parser, &m_settings, buf, len);
}