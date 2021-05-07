#include "Server.h"

//静态变量定义区
bool Server::stop;
unordered_map<Status, HttpResponse> Server::error_map;

//限制参数
int Server::thread_num;
int Server::queue_size;
int Server::max_fd;

//连接参数
int Server::port;
string Server::root;
int Server::listen_fd;

//redis参数
string Server::redis_ip;
int Server::redis_port;

//功能模块
Handler *Server::handler;
ThreadPool Server::thread_pool;
int Server::epoll_fd;
struct epoll_event *Server::ep_events;

//Log模块相关参数
string Server::log_path;
bool Server::info_on;
bool Server::debug_on;
bool Server::warn_on;
bool Server::error_on;

void Server::free()
{
	stop = true;
}

void Server::init()
{
	stop = false;
	Log::init(log_path, info_on, debug_on, warn_on, error_on);
	thread_pool.create(thread_num, queue_size);
	Log::log("server has inited", INFO);
	handler = new Handler[max_fd];
	HttpResponse::initContentTypeMap();
	error_map.clear();
	error_map.insert(make_pair(BAD_REQUEST, HttpResponse(BAD_REQUEST)));
	error_map.insert(make_pair(SERVICE_UNAVAILABLE, HttpResponse(SERVICE_UNAVAILABLE)));
	error_map.insert(make_pair(INTERNAL_SERVER_ERROR, HttpResponse(INTERNAL_SERVER_ERROR)));
	for (auto i = error_map.begin(); i != error_map.end(); i++)
	{
		i->second.generateResponse();
	}
}

void Server::loadConfig(string path)
{
	cout << " \033[32m[INFO]: \033[0m" << "Loading config" << endl;
	Config config;
	config.readFile(path.c_str());
	if (!config.lookupValue("server.thread_num", thread_num))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server thread_num setting not found, use default setting" << endl;
		thread_num = 8;
	}
	if (!config.lookupValue("server.queue_size", queue_size))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server queue_size setting not found, use default setting" << endl;
		queue_size = 1000;
	}
	if (!config.lookupValue("server.max_fd", max_fd))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server max_fd setting not found, use default setting" << endl;
		max_fd = 1024;
	}
	if (!config.lookupValue("server.root", root))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server root setting not found, use default setting" << endl;
		root = ".";
	}
	if (!config.lookupValue("server.redis_ip", redis_ip))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server redis_ip setting not found, use default setting" << endl;
		redis_ip = "127.0.0.1";
	}
	if (!config.lookupValue("server.redis_port", redis_port))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server redis_port setting not found, use default setting" << endl;
		redis_port = 6379;
	}
	if (!config.lookupValue("server.log_path", log_path))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server log_path setting not found, use default setting" << endl;
		log_path = "log/";
	}
	if (!config.lookupValue("server.info_on", info_on))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server info_on setting not found, use default setting" << endl;
		info_on = true;
	}
	if (!config.lookupValue("server.debug_on", debug_on))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server debug_on setting not found, use default setting" << endl;
		debug_on = true;
	}
	if (!config.lookupValue("server.warn_on", warn_on))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server warn_on setting not found, use default setting" << endl;
		warn_on = true;
	}
	if (!config.lookupValue("server.error_on", error_on))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server error_on setting not found, use default setting" << endl;
		error_on = true;
	}
	if (!config.lookupValue("server.port", port))
	{
		cout << " \033[32m[INFO]: \033[0m" << "server port setting not found, use default setting" << endl;
		port = 80;
	}
	redisContext *db = redisConnect(redis_ip.c_str(), redis_port);
	if (db->err != 0)
	{
		cout << " \033[32m[INFO]: \033[0m" << "redis is not running" << endl;
	}
	redisFree(db);
	cout << " \033[32m[INFO]: \033[0m" << "server runs in background" << endl;
}

void Server::start()
{
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw runtime_error("socket error");
	}
	int optval = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	if (bind(listen_fd, (const struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		throw runtime_error("bind error");
	}
	if (listen(listen_fd, 5) < 0)
	{
		throw runtime_error("listen error");
	}
	//设置listenfd为非阻塞
	int old_option = fcntl(listen_fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(listen_fd, F_SETFL, new_option);
	//初始化epoll
	epoll_fd = epoll_create(max_fd);
	struct epoll_event listen_ep_event;
	listen_ep_event.events = EPOLLIN | EPOLLET;
	listen_ep_event.data.fd = listen_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_ep_event);
	ep_events = new struct epoll_event[max_fd];
	Log::log("start loop", DEBUG);
	while (!stop)
	{
		int n_ready = epoll_wait(epoll_fd, ep_events, max_fd, 2 * 1000);
		for (int i = 0; i < n_ready; i++)
		{
			int temp_fd = ep_events[i].data.fd;
			if (temp_fd == listen_fd)
			{

				int connect_fd;
				while((connect_fd = accept(listen_fd, NULL, NULL)) > 0)
				{
					Log::log("accept", DEBUG);
					if (connect_fd >= max_fd)
					{
						close(connect_fd);
						Log::log("connect pool is full", INFO);
						//showError(connect_fd, SERVICE_UNAVAILABLE);
						continue;
					}
					else
					{
						handler[connect_fd].bind(connect_fd);
					}
				}
			}
			else if (ep_events[i].events & EPOLLIN)
			{
				try
				{
					handler[temp_fd].read();
					thread_pool.add(&handler[temp_fd]);
				}
				catch (const std::exception &e)
				{
					if (strcmp(e.what(), "server busy") == 0)
					{
						Log::log(e.what(), INFO);
						handler[temp_fd].debind();
						//showError(temp_fd, SERVICE_UNAVAILABLE);
					}
					else if (strcmp(e.what(), "client closed") == 0)
					{
						Log::log(e.what(), INFO);
						handler[temp_fd].debind();
					}
					else
					{
						Log::log(e.what(), ERROR);
						handler[temp_fd].debind();
						showError(temp_fd, INTERNAL_SERVER_ERROR);
					}
				}
			}
			else if (ep_events[i].events & EPOLLOUT)
			{
				try
				{
					handler[temp_fd].write();
				}
				catch (const std::exception &e)
				{
					Log::log(e.what(), ERROR);
					handler[temp_fd].debind();
				}
			}
			else if (ep_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				Log::log("client closed", INFO);
				handler[temp_fd].debind();
			}
		}
	}
	close(epoll_fd);
	close(listen_fd);
	Log::log("free the pool", DEBUG);
	thread_pool.free();
	Log::log("delete the handle", DEBUG);
	delete[] handler;
	Log::log("close server", INFO);
	Log::stop();
	while (!Log::finished());
}
//服务器忙时返回503是废案，后来发现服务器忙时直接关闭连接比发送错误码更现实：更不会占用宝贵的IO资源
void Server::showError(int connfd, Status st)
{
	if (error_map.find(st) != error_map.end())
	{
		send(connfd, error_map[st].getResponse(), error_map[st].getResponseSize(), 0);
	}
	handler[connfd].debind();
}
