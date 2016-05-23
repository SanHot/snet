//
//  main.cpp
//  snet
//
//  Created by San on 15/8/17.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include "IOLoop.h"
#include "http/HttpServer.h"
#include "tds/TdsWrapper.h"
#include "tds/unicode_utils.h"
#include "Buffer.h"

#ifdef WIN32
    #define DB_DRIVER "SQL SERVER"
#else
    #define DB_DRIVER "/usr/local/Cellar/freetds/0.95.80/lib/libtdsodbc.so"
#endif
#define MAX_CONTENT_LEN 0x100000

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
    std::string ret = "";
    try {
        char dsn[512] = {0};
        sprintf(dsn, "Driver={" DB_DRIVER "};Server=%s;Database=%s;UID=%s;PWD=%s;",
                si.server.c_str(), si.database.c_str(),si.uid.c_str(),si.pwd.c_str());
        nanodbc::connection conn(NANODBC_TEXT(dsn));
        long batch_size = 1;
        nanodbc::result row = execute(conn, NANODBC_TEXT(sql), batch_size);
        for (int i = 1; row.next(); ++i) {
            for(int j =0; j < row.columns(); ++j) {
                std::string col = "";
                if(!row.is_null(j))
                    col = row.get<nanodbc::string_type>(j);
                ret += col + "\t";
            }
            ret += ("\n");
        }
        content->write((void*)ret.c_str(), ret.length());
        content->buffer()[content->offset()] = '\0';
    }
    catch (std::runtime_error const& e) {
        ret.assign(e.what());
        content->write((void*)ret.c_str(), ret.length());
        content->buffer()[content->offset()] = '\0';
        return -1;
    }
    return 0;
}

void callback(uint8_t method, const std::string &url, HttpResponse *res) {
    if (url == "/") {
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_HTML);
        res->addHeader("Server", JOINTCOM_FLAG);
        res->setBody("<html><head><title>JOINTCOM</title></head>"
                             "<body><h1>Hello</h1>Now is 8:54"
                             "</body></html>");
    }
    else if (url == "/hello") {
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody("hello, world!\n");
    }
    else if (url == "/user") {
        //"10.204.118.102,1433";
        //"122.224.77.122,1433";
        //"jc#15User", "No19@Data", "original", "utf8"
        Buffer content;
        ServerInfo si("192.168.0.3,1433", "original", "jc#15User", "No19@Data");
        int ret = db_odbc_exec(si, "select * from tb_User", &content);
        if(ret == -1) {
            res->setHttp404Status(content.buffer());
            return;
        }
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody(content.buffer());
    }
    else if (url == "/query") {
        Buffer content;
        ServerInfo si("192.168.0.220,1433", "JointComV2", "sa", "jointwis");
        int ret = db_odbc_exec(si, "select KHDM, CPXH, KHXH, AddTime from tdb_bz_khtmdy "
                            "where KHDM <> ' ' and CPXH in "
                            "(select CPXH from tdb_cp_tsxx where XHBM = 'jcft252604n002-a1')", &content);
        if(ret == -1) {
            res->setHttp404Status(content.buffer());
            return;
        }
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody(content.buffer());
    }
    else {
        res->setHttp404Status();
    }
}

int main(int argc, const char * argv[]) {
    std::cout << "Hello, Jointcom!\n";

    auto a_loop = new IOLoop();
    auto a_svr = std::make_shared<HttpServer>(a_loop);
    a_svr->setHttpCallback(callback);
    a_svr->start("192.168.0.182", 8080, 2);
    a_loop->start_loop();
    return 0;
}
