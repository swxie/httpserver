#pragma once
#include <iostream>
#include <unordered_map>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <signal.h>
#include <sys/wait.h>
//#include <hiredis/hiredis.h>
#include <libconfig.h++>
#include <time.h>

#include "ThreadPool.h"
#include "http/Handler.h"
#include "http/HttpResponse.h"
#include "Log.h"

using namespace libconfig;
using namespace std;

class Handler;

class Server{
public:
    //用来初始化服务器的函数
    static void loadConfig(string path); //守护进程化前的初始化
    static void init(); //守护进程化后的初始化
    static void start();
    static void free();

    //用来获得信息的函数
    //static redisContext* getDb(){ return db; }
    static int getEpollFd(){ return epoll_fd; }
    static string getRoot(){ return root; }

private:
    //错误相关
    static void showError(int connfd, Status st);
    static unordered_map<Status, HttpResponse> error_map;
    
    //限制参数
    static int thread_num;
    static int queue_size;
    static int max_fd;

    //连接参数
    static int port;
    static string root;
    static int listen_fd;

    //redis参数
    //static string redis_ip;
    //static int redis_port;
    //static redisContext *db;
    
    //功能模块
    static Handler* handler;
    static ThreadPool thread_pool;
    static int epoll_fd;
    static struct epoll_event *ep_events;
    static bool stop;

    /*Log模块相关参数*/
    static string log_path;                                 
    static bool info_on;
    static bool debug_on;
    static bool warn_on;
    static bool error_on;
};