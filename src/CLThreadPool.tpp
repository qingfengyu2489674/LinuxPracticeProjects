#ifndef CLThreadPool_TPP
#define CLThreadPool_TPP

#include "CLThreadPool.h"

template<typename F, typename... Args>
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
