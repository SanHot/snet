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

//char addr1[] = "10.204.118.102:1433";
//char addr2[] = "122.224.77.122:1433";
//"jc#15User", "No19@Data", "original", "utf8"
int db_odbc_exec(const char *sql, char *content) {
    std::string ret = "";
    try {
        nanodbc::connection conn(NANODBC_TEXT("Driver={/usr/local/Cellar/freetds/0.95.80/lib/libtdsodbc.so};"
                                             "Server=122.224.77.122,1433;"
                                             "Database=original;UID=jc#15User;PWD=No19@Data;"));
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
        memcpy(content, ret.c_str(), ret.length());
    }
    catch (std::runtime_error const& e) {
//        cerr << e.what() << endl;
        return -1;
    }
    return 0;
}

void callback(uint8_t method, const std::string &url, HttpResponse *res) {
    if (url == "/") {
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_HTML);
        res->addHeader("Server", JOINTCOM_FLAG);
        res->setBody("<html><head><title>This is title</title></head>"
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
        char content[102400] = {0};
        int ret = db_odbc_exec("select * from tb_User", content);
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody(content);
    }
    else if (url == "/query") {
        char content[102400] = {0};
        int ret = db_odbc_exec("select  KHDM, CPXH, KHXH, AddTime from tdb_bz_khtmdy "
                            "where KHDM <> ' ' and CPXH in "
                            "(select CPXH from tdb_cp_tsxx where XHBM = 'jcft252604n002-a1')",
                    content);
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody(content);
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
    
    //a_svr->start("10.204.118.101", 8080, 3);
    //a_svr->start("192.168.3.8", 8080, 3);
    a_svr->start("127.0.0.1", 8080, 3);
    
    a_loop->start_loop();
    return 0;
}



