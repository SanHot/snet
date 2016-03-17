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

int main(int argc, const char * argv[]) {
    std::cout << "Hello, World!\n";
    
    auto a_loop = new IOLoop();
    auto a_svr = std::make_shared<HttpServer>(a_loop);
    a_svr->start("10.204.118.101", 8080);
    //a_svr->start("192.168.3.8", 8080);
    
    a_loop->start_loop();
    return 0;
}



