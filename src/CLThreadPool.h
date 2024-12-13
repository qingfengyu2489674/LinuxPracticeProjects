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

#include "CLCooridinator.h"

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


template <typename F, typename... Args>
std::future<typename std::result_of<F(Args...)>::type> CLThreadPool::enqueue(F&& func, Args&&... args) 
{
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>
    (
        std::bind(std::forward<F>(func), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        if (m_stop) 
        {
            throw std::runtime_error("enqueue on stopped CLThreadPool");
        }
        m_tasks.push([task]() 
        {
             (*task)(); 
        });
    }
    m_condition.notify_one();
    return res;
}


#endif
