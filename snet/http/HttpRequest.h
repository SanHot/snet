//
// Created by San on 16/6/16.
//

#ifndef SNET_HTTPREQUEST_H
#define SNET_HTTPREQUEST_H

#include "../stdafx.h"
#include "http_parser.h"

struct CaseKeyCmp {
    bool operator() (const std::string& p1, const std::string& p2) const {
        return strcasecmp(p1.c_str(), p2.c_str()) < 0;
    }
};
typedef std::map<std::string, std::string, CaseKeyCmp> Req_Headers_t;
typedef std::map<std::string, std::string> Req_Query_Params_t;

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();

    void clear();
    void parseUrl();
    std::string dump();
    void parseQuery();

//Header-key

    void setMethod(http_method method) {
        m_method = method;
    }
    http_method method() const { return m_method; }

    void setUrl(const char* content) {
        m_url.assign(content);
        parseUrl();
    }
    const std::string& url() const { return m_url; }

    void setBody(const char* content) {
        m_body.assign(content);
    }
    const std::string& body() const { return m_body; }

//url-param

    void setPath(const char* content) {
        m_url_path.assign(content);
    }
    const std::string& path() const { return m_url_path; }

    void setQuery(const char* content) {
        m_url_query.assign(content);
    }
    const std::string& query() const { return m_url_query; }
    const Req_Query_Params_t& query_param() const { return m_query_params; }

    void setSchema(const char* content) {
        m_url_schema.assign(content);
    }
    const std::string& schema() const { return m_url_schema; }

    void setPort(uint16_t port) {
        m_url_port = port;
    }
    uint16_t port() { return m_url_port; }

    void setFragment(const char* content) {
        m_url_fragment.assign(content);
    }
    const std::string& fragment() const { return m_url_fragment; }

//Header-param

    void addHeader(const char* key, const char* val) {
        std::string field(key);
        std::string value(val);
        while (!value.empty() && isspace(value[value.size()-1])) {
            value.resize(value.size()-1);
        }
        m_headers[field] = value;
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        auto it = m_headers.find(field);
        if (it != m_headers.end()) {
            result = it->second;
        }
        return result;
    }

private:
    std::vector<std::string> split(const std::string& src,
                                      const std::string& delim,
                                      size_t maxParts = size_t(-1));
    std::string unescape(const std::string& src);
    std::string   escape(const std::string & src);

private:
    //http 字段
    std::string m_url;
    std::string m_body;
    http_method m_method;
    unsigned short majorVersion;
    unsigned short minorVersion;
    Req_Headers_t m_headers;

    //url 格式化
    uint16_t m_url_port;
    std::string m_url_schema;
    std::string m_url_host;
    std::string m_url_path;
    std::string m_url_query;
    std::string m_url_fragment;
    //query 格式化
    Req_Query_Params_t m_query_params;
};


#endif //SNET_HTTPREQUEST_H
