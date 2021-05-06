#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <semaphore.h>


using namespace std;

enum Level {INFO , DEBUG , WARN , ERROR };

class Log{
public:
    static void init(string path, bool Info_on, bool Debug_on, 
                    bool Warn_on, bool Error_on);   //初始化Log模块, 启动工作线程
    static void log(string info, Level level);
    static void stop();
    static bool finished();
private:
    static string log_file_name;                    //日志文件名
    static queue<string> log_queue;                 //代写入文件的日志队列
    static mutex log_queue_mutex;
    static string log_path;
    static bool info_on;
    static bool debug_on;
    static bool warn_on;
    static bool error_on;
    static sem_t log_queue_sem;
    static bool stop_status;
    static bool finished_status;

    static void asyncWriteFile();                 //日志类的工作线程
};

class Time{
public:
    Time();
    string getDate();
    string getTime();
private:
    string time_str;
};