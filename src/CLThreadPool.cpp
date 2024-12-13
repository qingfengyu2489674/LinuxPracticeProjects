#include "CLThreadPool.h"

CLThreadPool::CLThreadPool(size_t threads) : m_numThreads(threads), m_stop(false) 
{
    for (size_t i = 0; i < threads; ++i) 
    {
        pthread_t worker;
        pthread_create(&worker, nullptr, WorkerThread, this);
        m_workers.push_back(worker);
    }
}

CLThreadPool::~CLThreadPool() 
{
    m_stop = true;
    m_condition.notify_all();
    for (auto& worker : m_workers) 
    {
        pthread_join(worker, nullptr);
    }
}

void* CLThreadPool::WorkerThread(void* arg) 
{
    CLThreadPool* pool = static_cast<CLThreadPool*>(arg);
    while (true) 
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(pool->m_queueMutex);

            pool->m_condition.wait(lock, [pool] 
            { 
                return pool->m_stop || !pool->m_tasks.empty(); 
            });

            if (pool->m_stop && pool->m_tasks.empty()) 
            {
                return nullptr; 
            }
            task = pool->m_tasks.front();
            pool->m_tasks.pop();
        }
        task();
    }
    return nullptr;
}

