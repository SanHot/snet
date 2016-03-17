//
//  stdafx_http.h
//  snet
//
//  Created by San on 16/3/2.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef stdafx_http_h
#define stdafx_http_h

#include "stdafx.h"

#define HTTP_RESPONSE_HEADER    "HTTP/1.1 200 OK\r\n"\
                                "Connection:close\r\n"\
                                "Content-Length:%d\r\n"\
                                "Content-Type:multipart/form-data\r\n\r\n"
#define HTTP_RESPONSE_EXTEND        "HTTP/1.1 200 OK\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:%d\r\n"\
                                    "Content-Type:multipart/form-data\r\n\r\n"
#define HTTP_RESPONSE_HTML          "HTTP/1.1 200 OK\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:%d\r\n"\
                                    "Content-Type:text/html;charset=utf-8\r\n\r\n%s"
#define HTTP_RESPONSE_HTML_MAX      1024
#define HTTP_RESPONSE_403           "HTTP/1.1 403 Access Forbidden\r\n"\
                                    "Content-Length: 0\r\n"\
                                    "Connection: close\r\n"\
                                    "Content-Type: text/html;charset=utf-8\r\n\r\n"
#define HTTP_RESPONSE_403_LEN       strlen(HTTP_RESPONSE_403)
#define HTTP_RESPONSE_404           "HTTP/1.1 404 Not Found\r\n"\
                                    "Content-Length: 0\r\n"\
                                    "Connection: close\r\n"\
                                    "Content-Type: text/html;charset=utf-8\r\n\r\n"
#define HTTP_RESPONSE_404_LEN       strlen(HTTP_RESPONSE_404)
#define HTTP_RESPONSE_500           "HTTP/1.1 500 Internal Server Error\r\n"\
                                    "Connection:close\r\n"\
                                    "Content-Length:0\r\n"\
                                    "Content-Type:text/html;charset=utf-8\r\n\r\n"
#define HTTP_RESPONSE_500_LEN       strlen(HTTP_RESPONSE_500)

#endif /* stdafx_http_h */
