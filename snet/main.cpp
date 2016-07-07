//
//  main.cpp
//  snet
//
//  Created by San on 15/8/17.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include <signal.h>
#include <vector>
#include "IOLoop.h"
#include "http/HttpServer.h"
#include "http/json.hpp"
#include "tds/unicode_utils.h"
#include "Buffer.h"

#ifdef WIN32
    #define DB_DRIVER "SQL SERVER"
#else
    #define DB_DRIVER "/usr/local/Cellar/freetds/0.95.80/lib/libtdsodbc.so"
#endif
#define MAX_CONTENT_LEN 0x100000
#define EXEC_CONFIG_FILE "exec_config"
#define NET_CONFIG_FILE "net_config"
#define DB_CONFIG_FILE "db_config"

#define FLAG_TXT 0
#define FLAG_JSON 1
#define FLAG_CSV 2

nlohmann::json g_si_config;
nlohmann::json g_exec_config;
Mutex g_mutex;

struct ServerInfo {
    ServerInfo() {}
    ServerInfo(const char* svr, const char* db, const char* user, const char* password) {
        server.assign(svr);
        database.assign(db);
        uid.assign(user);
        pwd.assign(password);
    }
    ServerInfo(std::string svr, std::string db, std::string user, std::string password) {
        server.assign(svr);
        database.assign(db);
        uid.assign(user);
        pwd.assign(password);
    }
    std::string server;
    std::string database;
    std::string uid;
    std::string pwd;
};

struct StatementsInfo {
    int flag;
    std::string sql;
    int param_count;
    nlohmann::json param;
    nlohmann::json content_type;
};

int readConfig(const char* path, nlohmann::json& config) {
    char b[10240] = {0};
    FILE *fp = fopen(path, "rb");
    if(fp == nullptr) {
        LOG_STDOUT("%s OpenError!", path);
        return -1;
    }
    fread(b, sizeof(char), 10240, fp);
    fclose(fp);
    try {
        config = nlohmann::json::parse(b);
    }
    catch(std::exception const& e) {
        LOG_STDOUT("readConfig(%s): %s", path, e.what());
        return -1;
    }
    return 0;
}

int readUrlConfig(const http_method& method, const std::string& path,
                  ServerInfo& svr_info, StatementsInfo& smt_info) {
    nlohmann::json si_config;
    nlohmann::json exec_config;
    {
        MUTEXGUARD_T(g_mutex);
        si_config = g_si_config;
        exec_config = g_exec_config;
    }

    //匹配path
    if(exec_config.find(path) == exec_config.end())
        return -1;

    try {
        nlohmann::json j_exec = exec_config[path];

        if(j_exec["exec"] == "query")
            smt_info.flag = HTTP_GET;
        else if(j_exec["exec"] == "exec")
            smt_info.flag = HTTP_POST;
        else if(j_exec["exec"] == "delete")
            smt_info.flag = HTTP_DELETE;
        else
            smt_info.flag = -1;

        //匹配method
        if(smt_info.flag != method) {
            LOG_STDOUT("readUrlConfig(%s): method can not match", path.c_str());
            return -1;
        }

        std::string si_name = j_exec["db"];
        nlohmann::json j_db = si_config[si_name];

        //获取server参数
        svr_info = ServerInfo(j_db["host"], j_db["db"], j_db["uid"], j_db["pwd"]);
        //获取exec参数
        smt_info.sql = j_exec["sql"];

        //获取param_count参数
        nlohmann::json count = j_exec["param_count"];
        if(!count.empty() && count.is_number())
            smt_info.param_count = count;
        else
            smt_info.param_count = 0;

        if(j_exec.find("param") != j_exec.end()) {
            smt_info.param = j_exec["param"];
        }

        if(j_exec.find("content_type") != j_exec.end()) {
            smt_info.content_type = j_exec["content_type"];
        }
    }
    catch(std::exception const& e) {
        LOG_STDOUT("readUrlConfig(%s): %s", path.c_str(), e.what());
        return -1;
    }

    return 0;
}

int db_odbc_exec(const ServerInfo &si, const StatementsInfo &smt_info, Buffer *content, int& content_flag) {
    try {
        char dsn[512] = {0};
        const char ct = ',';
        const char lf = '\n';
        const char* delim = ";";
        sprintf(dsn, "Driver={" DB_DRIVER "};Server=%s;Database=%s;UID=%s;PWD=%s;",
                si.server.c_str(), si.database.c_str(), si.uid.c_str(), si.pwd.c_str());
        nanodbc::connection conn(NANODBC_TEXT(dsn), 10);
        long batch_size = 1;
        LOG_STDOUT("DB_EXEC(%s): Start exec", si.database.c_str());
        nanodbc::result row = execute(conn, NANODBC_TEXT(smt_info.sql), batch_size, 30);
        if (smt_info.flag == HTTP_GET) {
            for (int n = 0; n < row.columns(); ++n) {
                std::string col_name = row.column_name(n);
                std::string::size_type pos;
                while ((pos = col_name.find_first_of(ct)) != std::string::npos)
                    col_name.replace(pos, 1, delim);
                content->write((void *) col_name.c_str(), col_name.length());
                if (n != row.columns() - 1)
                    content->write((void *) &ct, 1);
                else
                    content->write((void *) &lf, 1);
            }

            for (int i = 1; row.next(); ++i) {
                for (int n = 0; n < row.columns(); ++n) {
                    std::string col = "";
                    if (!row.is_null(n)) {
                        col = row.get<nanodbc::string_type>(n);
                        std::string::size_type pos;
                        while ((pos = col.find_first_of(ct)) != std::string::npos)
                            col.replace(pos, 1, delim);
                        content->write((void *) col.c_str(), col.length());
                    }
                    if (n != row.columns() - 1)
                        content->write((void *) &ct, 1);
                    else
                        content->write((void *) &lf, 1);
                }
            }
            LOG_STDOUT("DB_EXEC(%s): Fetch %d", si.database.c_str(), (int) content->offset());
            content->buffer()[content->offset()] = '\0';
            content_flag = FLAG_CSV;
            return 0;
        }
        else {
            int n = row.affected_rows();
            LOG_STDOUT("DB_EXEC(%s): Affected %d", si.database.c_str(), n);
            nlohmann::json result;
            result["result"] = n;
            std::string ret = result.dump();
            content->write((void *)ret.c_str(), ret.length());
            content->buffer()[content->offset()] = '\0';
            content_flag = FLAG_TXT;
            return n;
        }
    }
    catch (std::exception const& e) {
        std::string ret(e.what());
        content->write((void*)ret.c_str(), ret.length());
        content->buffer()[content->offset()] = '\0';
        return -1;
    }
}

void callback(const HttpRequest* req, HttpResponse* res) {
    Buffer content;
    ServerInfo svr_info;
    StatementsInfo smt_info;

    if (req->path() == "/") {
            res->setStatusCode(HttpResponse::HTTP_OK);
            res->setContentType(CONTEXT_TYPE_HTML);
            res->addHeader("Server", JOINTCOM_FLAG);
            char date[128] = {0};
            get_now(date);
            res->setBody("<html><head><title>JOINTCOM</title></head>"
                                 "<body><h1>Welcom to JOINTCOM</h1>Now is " + std::string(date) +
                         "</body></html>");
    }
    else if (readUrlConfig(req->method(), req->path(), svr_info, smt_info) == 0) {
        if (req->method() == HTTP_GET || req->method() == HTTP_DELETE) {
            if(smt_info.param_count > 0) {
                //固定参数
                //固定参数需匹配参数数量,不匹配时返回404
                if(smt_info.param_count != req->query_param().size()) {
                    res->setHttp404Status("Query-Param error!");
                    return;
                }
                auto it = req->query_param().begin();
                for (int i = 0; i < smt_info.param_count; i++) {
                    auto n = smt_info.sql.find_first_of('?');
                    if (n != std::string::npos) {
                        smt_info.sql.replace(n, 1, it->first);
                        it++;
                    }
                }
            }
            else if (smt_info.param_count == 0 && req->query_param().size() > 0) {
                //自由参数
                auto it = req->query_param().begin();
                std::string temp = " where ";
                for (int i = 0; i < req->query_param().size(); i++) {
                    temp += "[";
                    temp += it->first;
                    temp += "] = '";
                    temp += it->second;
                    temp += "'";
                    if (i != req->query_param().size() - 1) {
                        temp += " and ";
                        it++;
                    }
                }
                smt_info.sql += temp;
            }
            else if(smt_info.param_count == 0 && req->query_param().size() == 0 && req->method() == HTTP_DELETE) {
                //删除方法的参数不允许参数为0, 返回404
                res->setHttp404Status("Delete-Param error!");
                return;
            }
        }
        else if(req->method() == HTTP_POST) {
            std::string body = req->body();
            std::string sql = "";
            std::vector<std::string> vec = split_string(body, "\n");
            for(auto it = vec.begin(); it != vec.end(); it++) {
                std::string val = *it;
                if(val == "") continue;
                std::string temp = smt_info.sql;
                auto n = temp.find_first_of('?');
                if (n != std::string::npos)
                    temp.replace(n, 1, val.c_str());
                else {
                    res->setHttp404Status("Config-Sql error!");
                    return;
                }
                sql += temp + "\n";
            }
            smt_info.sql = sql;
        }

        int content_flag = 0;
        int ret = db_odbc_exec(svr_info, smt_info, &content, content_flag);
        if (ret == -1) {
            res->setHttp404Status(content.buffer());
            return;
        }
        res->setStatusCode(HttpResponse::HTTP_OK);
        if(content_flag == FLAG_JSON)
            res->setContentType(CONTEXT_TYPE_JSON);
        else if(content_flag == FLAG_CSV)
            res->setContentType(CONTEXT_TYPE_CSV);
        else
            res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", JOINTCOM_FLAG);
        res->setBody(content.buffer());
    }
    else {
            res->setHttp404Status();
    }
}

int main(int argc, const char * argv[]) {
    std::cout << "Hello, Jointcom!\n";

    nlohmann::json config = {{"server", "127.0.0.1"}, {"port", 8080}, {"thread", 1}};
    if(readConfig(NET_CONFIG_FILE, config)) {
        LOG_STDOUT( NET_CONFIG_FILE " Error!");
        return -1;
    }
    if(readConfig(DB_CONFIG_FILE, g_si_config)) {
        LOG_STDOUT( DB_CONFIG_FILE " Error!");
        return -1;
    }
    if(readConfig(EXEC_CONFIG_FILE, g_exec_config)) {
        LOG_STDOUT( EXEC_CONFIG_FILE " Error!");
        return -1;
    }

    auto a_loop = new IOLoop();
    auto a_svr = std::make_shared<HttpServer>(a_loop);
    a_svr->setHttpCallback(callback);
    std::string svr = config["server"];
    a_svr->start(svr.c_str(), config["port"], config["thread"]);

#if defined(SIGPIPE) && !defined(_WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

    a_loop->start_loop();
    return 0;
}
