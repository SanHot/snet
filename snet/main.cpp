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

int db_exec(const char *sql, char *content) {
    TdsWrapper helper;
//        char addr1[] = "10.204.118.102:1433";
//        char addr2[] = "122.224.77.122:1433";
//        char addr3[] = "192.168.0.3:1433";
    std::string ret;
    try {

            if (!helper.connect("192.168.0.220:1433", "sa", "jointwis", "JointComV2", "utf8")) {
//        if (!helper.connect("192.168.0.3:1433", "jc#15User", "No19@Data", "original", "utf8")) {
            printf("connect: error/n");
            return -1;
        }

        ret = "----------------------------------------\n";
        TdsQuery q(&helper);
        q.execQuery(sql);
        while (!q.eof()) {
            for (int i = 0; i < q.numFields(); ++i) {
                std::string temp;
                temp.assign(q.fieldValue(i));
                ret += temp + "\t";
            }
            ret += ("\n");
            q.nextRow();
        }
        q.finalize();
        ret += ("----------------------------------------\n");

        memcpy(content, ret.c_str(), ret.length());
    }
    catch (TdsException err) {
        ret = err.what();
    }
    helper.close();
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
        db_exec("select * from tb_User", content);
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType(CONTEXT_TYPE_PLAIN);
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody(content);
    }
    else if (url == "/query") {
        char content[102400] = {0};
        db_exec("select  KHDM, CPXH, KHXH, AddTime from tdb_bz_khtmdy "
                        "where KHDM <> ' ' and CPXH in "
                        "(select CPXH from tdb_cp_tsxx where XHBM = 'jcft252604n002-a1')",
                content);
//        db_exec("select * from tb_Switch", content);
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
    a_svr->start("192.168.0.182", 8080, 3);
    
    a_loop->start_loop();
    return 0;
}



