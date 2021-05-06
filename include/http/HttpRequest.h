#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Log.h"

using namespace std;

enum Status{
    READ_AGAIN = 0,
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500,
    SERVICE_UNAVAILABLE = 503
};

class HttpRequest{
public:
    Status parse(string &row_data);
    const string getMethod(){ return method; };
    const string& getUrl() const { return url; };
    const Status getStatus() {return status; };
    const map<string,string>& getHeader(){ return header; };
    const map<string,string>& getParams(){ return params; };

private:
    void resolve_get_params();      //解析GET请求参数

    Status status;                  //请求的结果
    string method;                  //该http请求方法
    string url;                     //请求URL
    string version;                 //http版本
    map<string,string> header;      //头部字段信息
    map<string,string> params;      //get请求参数,对post请求无效
    string req_body;                //post请求体,get请求则为空
};

std::vector<std::string> splitString(std::string srcStr, std::string delimStr,bool repeatedCharIgnored);