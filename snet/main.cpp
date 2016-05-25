//
//  main.cpp
//  snet
//
//  Created by San on 15/8/17.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include <signal.h>
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
#define NET_CONFIG_FILE "config"

struct ServerInfo {
    ServerInfo(const char* svr, const char* db, const char* user, const char* password) {
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

int db_odbc_exec(const ServerInfo& si, const char *sql, Buffer *content) {
    try {
        char dsn[512] = {0};
        char ct = ';';
        char lf = '\n';
        sprintf(dsn, "Driver={" DB_DRIVER "};Server=%s;Database=%s;UID=%s;PWD=%s;",
                si.server.c_str(), si.database.c_str(),si.uid.c_str(),si.pwd.c_str());
        nanodbc::connection conn(NANODBC_TEXT(dsn));
        long batch_size = 1;
        nanodbc::result row = execute(conn, NANODBC_TEXT(sql), batch_size);
        LOG_STDOUT("DB_EXEC(%s): Start exec", si.database.c_str());
        for (int i = 1; row.next(); ++i) {
            for(int j = 0; j < row.columns(); ++j) {
                std::string col = "";
                if(!row.is_null(j)) {
                    col = row.get<nanodbc::string_type>(j);
                    content->write((void*)col.c_str(), col.length());
                }
                if(j != row.columns()-1)
                    content->write((void*)&ct, 1);
                else
                    content->write((void*)&lf, 1);
            }
        }
        LOG_STDOUT("DB_EXEC(%s): Recv %d", si.database.c_str(), (int)content->offset());
        content->buffer()[content->offset()] = '\0';
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
    else if (url == "/user") {
        //"10.204.118.102,1433";
        //"122.224.77.122,1433";
        ServerInfo si("192.168.0.3,1433", "original", "jc#15User", "No19@Data");
        int ret = db_odbc_exec(si, "select * from tb_DFL_info", &content);
        if(ret == -1) {
            res->setHttp404Status(content.buffer());
            return;
        }
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", JOINTCOM_FLAG);
        res->setBody(content.buffer());
    }
    else if (url == "/pim") {
        ServerInfo si("192.168.0.253,1433", "TestPim", "pim", "!Q2w3e4r");
        int ret = db_odbc_exec(si, "select * from configtable", &content);
        if(ret == -1) {
            res->setHttp404Status(content.buffer());
            return;
        }
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType("text/plain");
        res->addHeader("Server", JOINTCOM_FLAG);
        res->setBody(content.buffer());
    }
    else if (url == "/query") {
        ServerInfo si("192.168.0.220,1433", "JointComV2", "sa", "jointwis");
        int ret = db_odbc_exec(si, "select KHDM, CPXH, KHXH, AddTime from tdb_bz_khtmdy "
                               "where KHDM <> ' ' and CPXH in "
                               "(select CPXH from tdb_cp_tsxx where XHBM = 'jcft252604n002-a1')"
                               , &content);
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
    if(isFileExist("config") != 0) {
        std::string s = config.dump();
        FILE *fp = fopen(NET_CONFIG_FILE, "wb");
        fwrite(s.c_str(), sizeof(char), s.length(), fp);
        fclose(fp);
    }
    else {
        char b[128] = {0};
        FILE *fp = fopen(NET_CONFIG_FILE, "rb");
        fread(b, sizeof(char), 128, fp);
        fclose(fp);
        try {
            config = nlohmann::json::parse(b);
        }
        catch(std::invalid_argument const& e) {
            std::cout << "Config File Error!\n";
            return -1;
        }
    }

    auto a_loop = new IOLoop();
    auto a_svr = std::make_shared<HttpServer>(a_loop);
    a_svr->setHttpCallback(callback);
    std::string svr = config["server"];
    a_svr->start(svr.c_str(), config["port"], config["thread"]);

#if defined(SIGPIPE) && !defined(_WIN32)
    (void)signal(SIGPIPE, SIG_IGN);
#endif

    a_loop->start_loop();
    return 0;
}
