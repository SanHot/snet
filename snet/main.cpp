//
//  main.cpp
//  snet
//
//  Created by San on 15/8/17.
//  Copyright (c) 2015年 ___SAN___. All rights reserved.
//

#include "IOLoop.h"
#include "IOStream.h"
#include "Log.h"
#include "TcpServer.h"
#include "http/HttpServer.h"

void callback(uint8_t method, const std::string& url, HttpResponse* res) {
    if (url == "/") {
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType("text/html");
        res->addHeader("Server", "Jointcom/snet1.0");
        //std::string now = Timestamp::now().toFormattedString();
        res->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello</h1>Now is 8:54"
                      "</body></html>");
    }
    else if (url == "/hello") {
        res->setStatusCode(HttpResponse::HTTP_OK);
        res->setContentType("text/plain");
        res->addHeader("Server", "Jointcom/snet1.0");
        res->setBody("hello, world!\n");
    }
    else {
        res->setHttp404Status();
    }
}

int main(int argc, const char * argv[]) {
    std::cout << "Hello, World!\n";
    
    auto a_loop = new IOLoop();
    auto a_svr = std::make_shared<HttpServer>(a_loop);
    a_svr->setHttpCallback(&callback);
    //a_svr->start("10.204.118.101", 8080);
    a_svr->start("192.168.3.8", 8080);
    
    a_loop->start_loop();
    return 0;
}



