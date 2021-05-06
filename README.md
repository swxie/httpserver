# httpserver
一个Linux下用C++实现的轻量级http服务器，用于学习网络编程

## 功能
- C/S模型，可以解析GET请求，并给客户端返回对应的静态资源
- 基于模拟proactor模型，使用epoll的ET模式、非阻塞IO和半同步半异步线程池实现高性能IO
- 实现了日志记录、配置文档和服务器守护进程化等服务器应该具有的功能
- 系统通过webbench测试，能在上千并发下保持0响应失败和和与低并发情况下一致的数据吞吐量

## Todo List
- 自己实现一个基于epoll的测试工具：webbench的fork模拟客户端很难满足高并发模拟的需要
- 通过redis连接池来实现页面的缓存功能
- 实现make install，进行服务器目录的生成


## 编译环境
- WSL1，Ubuntu 20.04 LTS
- C++ 11

## 构建工具
- make
- g++

## 库依赖
- libasan：用于检查内存错误
- libconfig++

## 编译和运行
首先保证库依赖已经全部安装, 库依赖对应的头文件都能被g++搜索到
在当前目录下执行以下命令来生成代码：
```
make all
```

生成后，该目录即是服务器的工作目录，其中config.cfg是服务器的配置文件，文件夹log是运行日志的目录，index.html是默认页面，httpserver是服务器程序

在管理员权限下，使用以下命令运行服务器，使之以守护进程方式的工作
```
sudo ./httpserver start
```
如果需要查询服务器是否正在运行，以及其对应的pid，可以使用如下命令
```
sudo ./httpserver status
```
而关闭服务器，则使用如下命令即可
```
sudo ./httpserver stop
```
该命令会发送SIGHUP给服务器，然后服务器进行资源回收，结束运行

## 压力测试
在WSL中，使用webbench进行压力测试，模拟1000个客户端进行10秒的并发访问，Webbench和服务器运行在同一平台上

回传32KB数据量应用（然而Webbench），测试环境均在本地

![image](https://user-images.githubusercontent.com/73411877/117260376-9a705780-ae81-11eb-88b7-92fe96f798a1.png)




