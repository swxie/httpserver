#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>

//#include "hiredis/hiredis.h"
#include "Log.h"
#include "http/HttpRequest.h"
#include "Server.h"

using namespace std;

class HttpRequest;

class HttpResponse{
public:
    //构造相关
    HttpResponse(Status st);
    HttpResponse();
    HttpResponse(const HttpResponse& resp);
    HttpResponse& operator=(const HttpResponse& resp);
    ~HttpResponse();
    static void initContentTypeMap();  //初始化映射表

    //处理请求
    void doRequest(HttpRequest *request); 
    void generateResponse();   //组装整个response，同时设置响应大小,被doRequest包含 

    //获取处理结果
    const char* getResponse();         //获取字节形式的response
    unsigned int getResponseSize();    //获取response字节数

private:
    //头部字段
    string Allow;
    string Content_Encoding;
    string Content_Length;
    string Content_Type;
    string Expires;
    string Last_Modified;
    string Location;
    string Refresh;
    string Set_Cookie;
    string WWW_Authenticate;
    string status;                              //http状态码 
    string response_body;                       //Http响应体
    string version;                             //http版本
    string date;                                //response生成时间
    string server;                              //http服务器名称
    map<string,string> custom_header;           //自定义头部字段

    //响应
    char* raw_response;                         //字节形式的原始response
    unsigned int raw_response_size;             //response总字节数

    //相关函数
    void autoSetContentType(string url);        //根据扩展名判断文件类型设置Content-Type字段
    void setHeader(string key, string val);     //设置头部自定义字段
    string generateHeader();                    //使用全部信息组装HTTP Response头部

    //类型表
    static map<string,string> content_type_map; //文件扩展名与Content-Type映射表
};

