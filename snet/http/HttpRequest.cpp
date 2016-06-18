//
// Created by San on 16/6/16.
//

#include "HttpRequest.h"
#include "../Log.h"

const char HEX2DEC[256] =
        {
                /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
                /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

                /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

                /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

                /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
        };

HttpRequest::HttpRequest()
{
    majorVersion = 1;
    minorVersion = 1;
    m_method = HTTP_GET;
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::clear() {
    majorVersion = 1;
    minorVersion = 1;
    m_method = HTTP_GET;

    m_body.clear();
    m_headers.clear();
    m_url.clear();
    m_url_port = 80;
    m_url_schema.clear();
    m_url_host.clear();
    m_url_path.clear();
    m_url_fragment.clear();
    m_url_query.clear();
    m_query_params.clear();
}

void HttpRequest::parseUrl() {
    if(m_url.empty())
        return;
    struct http_parser_url u;
    if(http_parser_parse_url(m_url.c_str(), m_url.size(), 0, &u) != 0) {
        LOG_STDERR("parseurl error %s", m_url.c_str());
        return;
    }
    //获取协议
    if(u.field_set & (1 << UF_SCHEMA)) {
        m_url_schema = m_url.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
    }
    //获取域名
    if(u.field_set & (1 << UF_HOST)) {
        m_url_host = m_url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
    }
    //获取端口
    if(u.field_set & (1 << UF_PORT)) {
        m_url_port = u.port;
    }
    else {
        //比较协议
        if(strcasecmp(m_url_schema.c_str(), "https") == 0 ||
                strcasecmp(m_url_schema.c_str(), "wss") == 0) {
            m_url_port = 443;
        }
        else {
            m_url_port = 80;
        }
    }
    //获取路径
    if(u.field_set & (1 << UF_PATH)) {
        m_url_path = m_url.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
    }
    //获取参数
    if(u.field_set & (1 << UF_QUERY)) {
        m_url_query = m_url.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
        parseQuery();
    }
    //获取锚
    if(u.field_set & (1 << UF_FRAGMENT)) {
        m_url_fragment = m_url.substr(u.field_data[UF_FRAGMENT].off, u.field_data[UF_FRAGMENT].len);
    }

}

void HttpRequest::parseQuery() {
    if(m_url_query.empty() || !m_query_params.empty())
        return;

    static std::string sep1 = "&";
    static std::string sep2 = "=";

    std::vector<std::string> args = split(m_url_query, sep1);
    std::string key;
    std::string value;
    for(size_t i = 0; i < args.size(); ++i) {
        std::vector<std::string> p = split(args[i], sep2);
        if(p.size() == 2) {
            key = p[0];
            value = p[1];
        }
        else if(p.size() == 1) {
            key = p[0];
//            value = "";
            value = p[0];
        }
        else {
            //invalid, ignore
            continue;
        }

        m_query_params.insert(std::make_pair(unescape(key), unescape(value)));
    }

}

std::string HttpRequest::dump() {
    static const std::string HostKey = "Host";
    static const std::string ContentLengthKey = "Content-Length";

    std::string str;
    parseUrl();
    char buf[1024];
    int n = 0;

    if(m_url_path.empty()) {
        m_url_path = "/";
    }
    if(m_url_query.empty()) {
        n = snprintf(buf, sizeof(buf), "%s %s HTTP/%d.%d\r\n",
                     http_method_str(m_method), m_url_path.c_str(), majorVersion, minorVersion);
    }
    else {
        n = snprintf(buf, sizeof(buf), "%s %s?%s HTTP/%d.%d\r\n",
                     http_method_str(m_method), m_url_path.c_str(), m_url_query.c_str(), majorVersion, minorVersion);
    }
    str.append(buf, n);
//    if(m_url_port == 80 || m_url_port == 443) {
//        m_headers[HostKey] = m_url_host;
//    }
//    else {
//        n = snprintf(buf, sizeof(buf), "%s:%d", m_url_host.c_str(), m_url_port);
//        m_headers[HostKey] = std::string(buf, n);
//    }
    if(m_headers.find(HostKey) == m_headers.end()) {
        m_headers[HostKey] = m_url_host;
    }
    if(m_method == HTTP_POST || m_method == HTTP_PUT) {
        n = snprintf(buf, sizeof(buf), "%d", int(m_body.size()));
        m_headers[ContentLengthKey] =  std::string(buf, n);
    }
    for(auto iter = m_headers.begin();iter!=m_headers.end();++iter) {
        n = snprintf(buf, sizeof(buf), "%s: %s\r\n", iter->first.c_str(), iter->second.c_str());
        str.append(buf, n);
    }

    str.append("\r\n");
    str.append(m_body);

    return str;
}

std::vector<std::string> HttpRequest::split(const std::string& src,
                                               const std::string& delim,
                                               size_t maxParts) {
    if(maxParts == 0) {
        maxParts = size_t(-1);
    }
    size_t lastPos = 0;
    size_t pos = 0;
    size_t size = src.size();

    std::vector<std::string> tokens;

    while(pos < size) {
        pos = lastPos;
        while(pos < size && delim.find_first_of(src[pos]) == std::string::npos) {
            ++pos;
        }

        if(pos - lastPos > 0) {
            if(tokens.size() == maxParts - 1) {
                tokens.push_back(src.substr(lastPos));
                break;
            }
            else {
                tokens.push_back(src.substr(lastPos, pos - lastPos));
            }
        }

        lastPos = pos + 1;
    }

    return tokens;
}

std::string HttpRequest::unescape(const std::string& src)
{
    //for a esacep character, is %xx, min size is 3
    if(src.size() <= 2) {
        return src;
    }

    // Note from RFC1630:  "Sequences which start with a percent sign
    // but are not followed by two hexadecimal characters (0-9, A-F) are reserved
    // for future extension"

    std::string dest(src.size(), '\0');

    size_t i = 0;
    size_t j = 0;
    while(i < src.size() - 2) {
        if(src[i] == '%') {
            char dec1 = HEX2DEC[src[i + 1]];
            char dec2 = HEX2DEC[src[i + 2]];
            if(dec1 != -1 && dec2 != -1) {
                dest[j++] = (dec1 << 4) + dec2;
                i += 3;
                continue;
            }
        }

        dest[j++] = src[i++];
    }

    while(i < src.size()) {
        dest[j++] = src[i++];
    }

    dest.resize(j);

    return dest;
}
