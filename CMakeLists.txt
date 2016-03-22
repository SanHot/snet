cmake_minimum_required(VERSION 3.4)
project(snet)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    snet/http/http_parser.c
    snet/http/http_parser.h
    snet/http/HttpConnection.h
    snet/http/HttpParserObj.cpp
    snet/http/HttpParserObj.h
    snet/http/HttpResponse.h
    snet/http/HttpServer.cpp
    snet/http/HttpServer.h
    snet/http/stdafx_http.h
    snet/http/TreadPool.cpp
    snet/http/TreadPool.h
    snet/BaseSocket.cpp
    snet/BaseSocket.h
    snet/Buffer.h
    snet/Callback.h
    snet/IOEvent.cpp
    snet/IOEvent.h
    snet/IOKevent.cpp
    snet/IOKevent.h
    snet/IOLoop.cpp
    snet/IOLoop.h
    snet/IPAddress.h
    snet/Log.cpp
    snet/Log.h
    snet/main.cpp
    snet/MutexGuard.cpp
    snet/MutexGuard.h
    snet/stdafx.h
    snet/stdafx_net.h
    snet/TcpServer.h
    snet/TcpStream.cpp
    snet/TcpStream.h
    snet/Util.cpp
    snet/Util.h snet/IOSelect.cpp snet/IOSelect.h)

add_executable(snet ${SOURCE_FILES})
if (WIN32)
    target_link_libraries(snet "ws2_32.lib")
endif (WIN32)