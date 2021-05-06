#include "ThreadPool.h"


ThreadPool::ThreadPool() :
    thread_count(0), queue_size(0),finished(false){
        lock.unlock();
        sem_init(&sem, 0, 0);
}



void ThreadPool::create(int thread_count, int queue_size)
{
    if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size < 0 || queue_size > MAX_QUEUE) 
    {
        thread_count = MAX_THREADS;
        queue_size = MAX_QUEUE;
    }
    this->thread_count = thread_count;
    this->queue_size = queue_size;
    threads.resize(thread_count);
    for (int i = 0; i < thread_count; i++)
        threads[i] = thread(work, this);
}

void ThreadPool::add(ThreadPoolTask* new_task)
{
    if (queue.size() >= queue_size)
        throw runtime_error("server busy");
    lock.lock();
    queue.push_back(new_task);
    lock.unlock();
    sem_post(&sem);
}


void ThreadPool::work(ThreadPool *thread_pool)
{
    while (!thread_pool->finished)
    {
        ThreadPoolTask* task;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        if (sem_timedwait(&thread_pool->sem, &ts) == 0){
            thread_pool->lock.lock();
            task = thread_pool->queue.front();
            thread_pool->queue.pop_front();
            thread_pool->lock.unlock();
            try
            {
                task->run();
            }
            catch(const exception& e)
            {
                Log::log(e.what(), ERROR);
            }
        }
    }
}

void ThreadPool::free(){
    finished = true;
    for (int i = 0; i < threads.size(); i++){
        try
        {
            threads[i].join();
        }
        catch(const std::exception& e)
        {
           Log::log(e.what(), ERROR);
        }
    }
}