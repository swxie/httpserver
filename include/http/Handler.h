#pragma once

#include <memory>
#include <iostream>

//#include "hiredis/hiredis.h"
#include "ThreadPool.h"
#include "sys/socket.h"
#include "sys/epoll.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "fcntl.h"

#include "Server.h"
#include "Log.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "Server.h"

using namespace std;

enum HandlerState {READ, WRITE};

class Server;
class HttpRequest;
class HttpResponse;

class Handler : public ThreadPoolTask{
public:
    Handler(){
        http_request = NULL;
        http_response = NULL;
    }
    ~Handler(){
        delete http_response;
        delete http_request;
    }
    //IO相关方法
    void bind(int sockfd);
    void debind();
    void read();
    void write();
    //调用方法
    void run();
    
private:
    //处理http响应需要的类
    HttpRequest* http_request;
    HttpResponse* http_response;

    //私有资源
    int sockfd;
    HandlerState state;
    string read_buffer;
    string write_buffer;
    int write_index;

};
