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
#include "tds/TdsWrapper.h"
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

#define FLAG_QUERY 0
#define FLAG_EXEC 1

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

int readConfig(const char* path, nlohmann::json& config) {
    char b[1024] = {0};
    FILE *fp = fopen(path, "rb");
    if(fp == nullptr) {
        LOG_STDOUT("%s OpenError!", path);
        return -1;
    }
    fread(b, sizeof(char), 1024, fp);
    fclose(fp);
    try {
        config = nlohmann::json::parse(b);
    }
    catch(std::invalid_argument const& e) {
        LOG_STDOUT("%s Incomplete!", path);
        return -1;
    }
    return 0;
}

int readUrlConfig(const std::string& url, ServerInfo& si, int& flag, std::string& sql) {
    nlohmann::json si_config;
    nlohmann::json exec_config;
    {
        MUTEXGUARD_T(g_mutex);
        si_config = g_si_config;
        exec_config = g_exec_config;
    }

    if(exec_config.find(url) == exec_config.end())
        return -1;

    try {
        nlohmann::json j_exec = exec_config[url];
        std::string si_name = j_exec["db"];
        nlohmann::json j_db = si_config[si_name];

        si = ServerInfo(j_db["host"], j_db["db"], j_db["uid"], j_db["pwd"]);
        flag = j_exec["exec"] == "query"? FLAG_QUERY:FLAG_EXEC;
        sql = j_exec["sql"];
    }
    catch(std::domain_error const& e) {
        LOG_STDOUT("readUrlConfig(%s): %s", url.c_str(), e.what());
        return -1;
    }

    return 0;
}

int db_odbc_exec(const ServerInfo &si, int flag, const char *sql, Buffer *content) {
    try {
        char dsn[512] = {0};
        char ct = ';';
        char lf = '\n';
        sprintf(dsn, "Driver={" DB_DRIVER "};Server=%s;Database=%s;UID=%s;PWD=%s;",
                si.server.c_str(), si.database.c_str(), si.uid.c_str(), si.pwd.c_str());
        nanodbc::connection conn(NANODBC_TEXT(dsn));
        long batch_size = 1;
        LOG_STDOUT("DB_EXEC(%s): Start exec", si.database.c_str());
        nanodbc::result row = execute(conn, NANODBC_TEXT(sql), batch_size);
        if (flag == FLAG_QUERY) {
            for (int i = 1; row.next(); ++i) {
                for (int j = 0; j < row.columns(); ++j) {
                    std::string col = "";
                    if (!row.is_null(j)) {
                        col = row.get<nanodbc::string_type>(j);
                        content->write((void *) col.c_str(), col.length());
                    }
                    if (j != row.columns() - 1)
                        content->write((void *) &ct, 1);
                    else
                        content->write((void *) &lf, 1);
                }
            }
            LOG_STDOUT("DB_EXEC(%s): Recv %d", si.database.c_str(), (int) content->offset());
            content->buffer()[content->offset()] = '\0';
        }
    }
    catch (std::runtime_error const& e) {
        std::string ret(e.what());
        content->write((void*)ret.c_str(), ret.length());
        content->buffer()[content->offset()] = '\0';
        return -1;
    }
    return 0;
}

void callback(uint8_t method, const std::string &url, HttpResponse *res) {
    Buffer content;
    std::string sql;
    ServerInfo si;
    int flag;

    if (url == "/") {
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_HTML);
        res->addHeader("Server", JOINTCOM_FLAG);
        char date[128] = {0};
        get_now(date);
        res->setBody("<html><head><title>JOINTCOM</title></head>"
                             "<body><h1>Welcom to JOINTCOM</h1>Now is " + std::string(date) +
                     "</body></html>");
    }
    else if(readUrlConfig(url, si, flag, sql) == 0) {
        int ret = db_odbc_exec(si, flag, sql.c_str(), &content);
        if(ret == -1) {
            res->setHttp404Status(content.buffer());
            return;
        }
        res->setStatusCode(HttpResponse::HTTP_OK);
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
