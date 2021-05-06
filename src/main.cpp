#include "unistd.h"
#include "signal.h"

#include "Server.h"
#include "Log.h"

#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

Server server;

//用于接收HUP信号，终止服务器运行的信号处理函数
void closeServer(int sig){
    server.free();
}

int main(int argc, char* argv[]){
    if (argc != 2){
        cerr << "please input the arg start/stop/status to continue" << endl;
        return -1;
    }
    //验证是否有超级用户权限
    if (getuid() != 0){
        cerr << "please use sudo to run the serve" << endl;
        return -1;
    }
    //服务器运行
    if (string(argv[1]) == "start"){
        //检查服务器是否正在运行
        ifstream ipidf("/var/run/httpserver.pid");
        if (ipidf.is_open()){
            int pid;
            ipidf >> pid;
            ipidf.close();
            kill(pid, SIGHUP);
            if (remove("/var/run/httpserver.pid") < 0){
                cerr << "httpserver is already running" << endl;
                return -1;
            }
            else
                cout << "restart the server" << endl;
        }
        //初始化
        try
        {
            server.loadConfig("./config.cfg");
            daemon(1, 1);
            server.init();
            ofstream pidf("/var/run/httpserver.pid");
            pidf << getpid() << endl;
            pidf.close();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return -1;
        }
        //相关系统设置
        system("ulimit -Sn 16394");
        struct sigaction sa;
        memset(&sa, '\0', sizeof(sa));
        sa.sa_handler = closeServer;
        sigfillset(&sa.sa_mask);
        if (sigaction(SIGHUP, &sa, NULL) == -1){
            Log::log("signal fault", ERROR);
            return -1;
        }
        Log::log("start the server", DEBUG);
        try
        {
            server.start();
        }
        catch(const std::exception& e)
        {
            Log::log(e.what(), ERROR);
            return -1;
        }
        return 0;
    }
    //服务器终止
    if (string(argv[1]) == "stop"){
        ifstream pidf("/var/run/httpserver.pid");
        if (!pidf.is_open()){
            cerr << "httpserver is not running" << endl;
            return -1;
        }
        int pid;
        pidf >> pid;
        pidf.close();
        kill(pid, SIGHUP);
        if (remove("/var/run/httpserver.pid") < 0){
                cerr << "remove file fault" << endl;
                return -1;
        }
        cout << "server in pid " << pid << " has finished" << endl;
        return 0;
    }
    //查询服务器状态
    if (string(argv[1]) == "status"){
        ifstream pidf("/var/run/httpserver.pid");
        if (!pidf.is_open()){
            cout << "httpserver is not running" << endl;
            return -1;
        }
        int pid;
        pidf >> pid;
        pidf.close();
        cout << "httpserver is running with pid " << pid << endl;
        return 0;
    }
}
