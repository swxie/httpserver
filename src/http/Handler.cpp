#include "http/Handler.h"

void Handler::bind(int sockfd)
{
    this->sockfd = sockfd;
    read_buffer.clear();
    write_buffer.clear();
    write_index = 0;
    int old_option = fcntl(sockfd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(sockfd, F_SETFL, new_option);
    state = READ;
    http_request = new HttpRequest();
    struct epoll_event temp_ep_event;
    temp_ep_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
    temp_ep_event.data.fd = sockfd;
    epoll_ctl(Server::getEpollFd(), EPOLL_CTL_ADD, sockfd, &temp_ep_event);
}

void Handler::debind()
{
    Log::log(string("debind fd ") + to_string(sockfd), DEBUG);
    delete http_response;
    http_response = nullptr;
    delete http_request;
    http_request = nullptr;
    epoll_ctl(Server::getEpollFd(), EPOLL_CTL_DEL, sockfd, 0);
    close(sockfd);
}

void Handler::run()
{
    if (state == READ)
    {
        //进行解析
        Status read_state = http_request->parse(read_buffer);
        if (read_state == BAD_REQUEST){
            throw runtime_error("bad request");
        }
        else if (read_state == READ_AGAIN){
            struct epoll_event temp_ep_event;
            temp_ep_event.events  = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
            temp_ep_event.data.fd = sockfd;
            epoll_ctl(Server::getEpollFd(), EPOLL_CTL_MOD, sockfd, &temp_ep_event);
            return;
        }
        http_response = new HttpResponse(read_state);
        http_response->doRequest(http_request);
        write_buffer = string(http_response->getResponse());
        state = WRITE;
        struct epoll_event temp_ep_event;
        temp_ep_event.events = EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
        temp_ep_event.data.fd = sockfd;
        epoll_ctl(Server::getEpollFd(), EPOLL_CTL_MOD, sockfd, &temp_ep_event);
        return;
    }
}

void Handler::read()
{
    int byte_read = 0;
    char buffer[1000];
    while (1)
    {
        byte_read = recv(sockfd, buffer, 900, 0);
        if (byte_read == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                throw runtime_error("system error");
        }
        else if (byte_read == 0)
            throw runtime_error("client closed");
        read_buffer += buffer;
    }
}

void Handler::write()
{
    if (write_index >= write_buffer.size())
    {
        bind(sockfd);
        return;
    }
    while (1)
    {
        int temp = send(sockfd, write_buffer.c_str() + write_index, write_buffer.size() - write_index, 0);
        if (temp < 0)
        {
            if (errno == EAGAIN)
            {
                struct epoll_event temp_ep_event;
                temp_ep_event.events = EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
                temp_ep_event.data.fd = sockfd;
                epoll_ctl(Server::getEpollFd(), EPOLL_CTL_MOD, sockfd, &temp_ep_event);
                break;
            }
            else
                throw runtime_error(strerror(errno));
        }
        write_index += temp;
        if (write_index >= write_buffer.size())
        {
           debind();
           break;
        }
    }
}
