#pragma once
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <iostream>
#include <semaphore.h>

#include <Log.h>

const int MAX_THREADS = 32;
const int MAX_QUEUE = 65535;

using namespace std;

class ThreadPoolTask
{
public:
    virtual void run() = 0;
};


class ThreadPool
{
private:
    //锁参数
    mutex lock;
    sem_t sem;
    //线程池参数
    vector<thread> threads;
    int thread_count;
    //工作队列参数
    deque<ThreadPoolTask *> queue;
    int queue_size;

    bool finished;
    static void work(ThreadPool *thread_pool);
public:
    ThreadPool();
    void create(int thread_count, int queue_size);
    void add(ThreadPoolTask *new_task);
    void free();
};
