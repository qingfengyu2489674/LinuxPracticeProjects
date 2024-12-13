#ifndef CLThreadPool_H
#define CLThreadPool_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <stdexcept>

#include "../CLCooridinator.h"

class CLThreadPool 
{
public:
    CLThreadPool(size_t numThreads);
    ~CLThreadPool();

    template<typename F, typename... Args>
    std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& func, Args&&... args);

private:
    static void* WorkerThread(void* arg);

private:
    size_t m_numThreads;
    std::atomic<bool> m_stop;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable_any m_condition;
    std::vector<pthread_t> m_workers;
};

#include "CLThreadPool.tpp"  
#endif
