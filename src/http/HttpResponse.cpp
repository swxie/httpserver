#include "http/HttpResponse.h"

map<string, string> HttpResponse::content_type_map;

string GetGmtTime()
{
    time_t rawTime;
    struct tm *timeInfo;
    char szTemp[30] = {0};
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    strftime(szTemp, sizeof(szTemp), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    string GmtTime(szTemp);

    return GmtTime;
}

HttpResponse::HttpResponse(Status st)
{
    version = "HTTP/1.1";
    status = st;
    date = GetGmtTime();
    server = "httpserver";
    raw_response = NULL;
    raw_response_size = 0;
}

HttpResponse::HttpResponse()
{
    version = "HTTP/1.1";
    status = OK;
    date = GetGmtTime();
    server = "httpserver";
    raw_response = NULL;
    raw_response_size = 0;
}

HttpResponse::HttpResponse(const HttpResponse &resp)
{
    Allow = resp.Allow;
    Content_Encoding = resp.Content_Encoding;
    Content_Length = resp.Content_Length;
    Content_Type = resp.Content_Type;
    Expires = resp.Expires;
    Last_Modified = resp.Last_Modified;
    Location = resp.Location;
    Refresh = resp.Refresh;
    Set_Cookie = resp.Set_Cookie;
    WWW_Authenticate = resp.WWW_Authenticate;

    version = resp.version;
    status = resp.status;
    date = resp.date;
    server = resp.server;
    custom_header = resp.custom_header;
    response_body = resp.response_body;
    raw_response_size = resp.raw_response_size;

    raw_response = NULL;
    if (resp.raw_response != NULL)
    {
        raw_response = new char[raw_response_size + 1];
        memcpy(raw_response, resp.raw_response, raw_response_size + 1);
    }
}

HttpResponse &HttpResponse::operator=(const HttpResponse &resp)
{
    Allow = resp.Allow;
    Content_Encoding = resp.Content_Encoding;
    Content_Length = resp.Content_Length;
    Content_Type = resp.Content_Type;
    Expires = resp.Expires;
    Last_Modified = resp.Last_Modified;
    Location = resp.Location;
    Refresh = resp.Refresh;
    Set_Cookie = resp.Set_Cookie;
    WWW_Authenticate = resp.WWW_Authenticate;

    version = resp.version;
    status = resp.status;
    date = resp.date;
    server = resp.server;
    custom_header = resp.custom_header;
    response_body = resp.response_body;
    raw_response_size = resp.raw_response_size;

    raw_response = NULL;
    if (resp.raw_response != NULL)
    {
        raw_response = new char[raw_response_size + 1];
        memcpy(raw_response, resp.raw_response, raw_response_size + 1);
    }
    return *this;
}

HttpResponse::~HttpResponse()
{
    delete[] raw_response;
}

void HttpResponse::initContentTypeMap()
{
    content_type_map.insert(pair<string, string>("html", "text/html"));
    content_type_map.insert(pair<string, string>("htm", "text/html"));
    content_type_map.insert(pair<string, string>("shtml", "text/html"));
    content_type_map.insert(pair<string, string>("css", "text/css"));
    content_type_map.insert(pair<string, string>("js", "text/javascript"));
    content_type_map.insert(pair<string, string>("txt", "text/plain"));
    content_type_map.insert(pair<string, string>("js", "text/javascript"));
    content_type_map.insert(pair<string, string>("xml", "text/xml"));

    content_type_map.insert(pair<string, string>("ico", "image/x-icon"));
    content_type_map.insert(pair<string, string>("jpg", "image/jpeg"));
    content_type_map.insert(pair<string, string>("jpeg", "image/jpeg"));
    content_type_map.insert(pair<string, string>("jpe", "image/jpeg"));
    content_type_map.insert(pair<string, string>("gif", "image/gif"));
    content_type_map.insert(pair<string, string>("png", "image/png"));
    content_type_map.insert(pair<string, string>("tiff", "image/tiff"));
    content_type_map.insert(pair<string, string>("tif", "image/tiff"));
    content_type_map.insert(pair<string, string>("rgb", "image/x-rgb"));

    content_type_map.insert(pair<string, string>("mpeg", "video/mpeg"));
    content_type_map.insert(pair<string, string>("mpg", "video/mpeg"));
    content_type_map.insert(pair<string, string>("mpe", "video/mpeg"));
    content_type_map.insert(pair<string, string>("qt", "video/quicktime"));
    content_type_map.insert(pair<string, string>("mov", "video/quicktime"));
    content_type_map.insert(pair<string, string>("avi", "video/x-msvideo"));
    content_type_map.insert(pair<string, string>("movie", "video/x-sgi-movie"));

    content_type_map.insert(pair<string, string>("woff", "application/font-woff"));
    content_type_map.insert(pair<string, string>("ttf", "application/octet-stream"));
}

void HttpResponse::setHeader(string key, string val)
{
    custom_header.insert(pair<string, string>(key, val));
}

void HttpResponse::doRequest(HttpRequest *request)
{
    string url = request->getUrl();
    status = to_string(request->getStatus());
    if (status != "200")
    {
        generateResponse();
        return;
    }
    if (url == "/")
        url = Server::getRoot() + '/' + "index.html";
    else
        url = Server::getRoot() + request->getUrl();
    Log::log("request url = " + url, INFO);
    //使用缓存
    //auto db = Server::getDb();
    //redisReply *reply = (redisReply *)redisCommand(db, "%s exists", url.c_str());
    //if (reply->integer == 1)
    //{
    //    reply = (redisReply *)redisCommand(db, "get %s", url.c_str());
    //    response_body = string(reply->str);
    //    generateResponse();
    //    return;
    //}
    ifstream in_file(url);
    if (in_file.fail())
    {
        Log::log("[404]: " + url + " not found", WARN);
        status = "404";
        generateResponse();
        return;
    }
    autoSetContentType(url);
    stringstream buffer;
    buffer << in_file.rdbuf();
    response_body = buffer.str();
    //redisCommand(db, "set %s %s", url.c_str(), response_body.c_str());
    in_file.close();
    generateResponse();
}

const char *HttpResponse::getResponse()
{
    return raw_response;
}

unsigned int HttpResponse::getResponseSize()
{
    return raw_response_size;
}

void HttpResponse::autoSetContentType(string url)
{
    size_t pos = url.find_last_of('.');
    if (pos == string::npos)
    {
        return;
    }
    string ext = url.substr(pos + 1);
    Log::log("file ext = " + ext, DEBUG);
    auto iter = content_type_map.find(ext);
    if (iter != content_type_map.end())
    {
        Content_Type = iter->second;
    }
}

//生成完整响应
void HttpResponse::generateResponse()
{
    string header(generateHeader());
    string response = header + response_body;
    raw_response = new char[response.size() + 1];
    if (response.size() == 0 || status == "404")
    {
        Log::log("In HttpResponse.cpp: raw_response is NULL", INFO);
        memcpy(raw_response, response.c_str(), response.size() + 1);
        raw_response_size = response.size();
        return;
    }
    memcpy(raw_response, response.c_str(), response.size() + 1);
    raw_response_size = response.size();
    return;
}

string HttpResponse::generateHeader()
{
    string header;
    header += (version + " ");
    header += (status + "\n");
    (header += "Date:") += (date + "\n");
    (header += "Server:") += (server + "\n");

    if (Allow.size() != 0)
        (header += "Allow:") += (Allow + "\n");
    if (Content_Encoding.size() != 0)
        (header += "Content-Encoding:") += (Content_Encoding + "\n");
    if (Content_Length.size() != 0)
        (header += "Content-Length:") += (Content_Length + "\n");
    if (Content_Type.size() != 0)
        (header += "Content-Type:") += (Content_Type + "\n");
    if (Expires.size() != 0)
        (header += "Expires:") += (Expires + "\n");
    if (Last_Modified.size() != 0)
        (header += "Last-Modified:") += (Last_Modified + "\n");
    if (Location.size() != 0)
        (header += "Location:") += (Location + "\n");
    if (Refresh.size() != 0)
        (header += "Refresh:") += (Refresh + "\n");
    if (Set_Cookie.size() != 0)
        (header += "Set-Cookie:") += (Set_Cookie + "\n");
    if (WWW_Authenticate.size() != 0)
        (header += "WWW-Authenticate:") += (WWW_Authenticate + "\n");

    //基本字段添加完成后，填充用户自定义字段
    for (auto i : custom_header)
    {
        ((header += i.first) += ":") += i.second += "\n";
    }

    header += "\r\n";
    return header;
}