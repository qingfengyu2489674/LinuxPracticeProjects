// my_mutex.h
#pragma once

#include "my_futex_utils.h"

namespace MyPthreadImpl {

class MyMutex {
private:
    std::atomic<int> futex_state;

public:
    MyMutex();
    ~MyMutex() = default;

    MyMutex(const MyMutex&) = delete;
    MyMutex& operator=(const MyMutex&) = delete;

    void lock();
    void unlock();
};

} // namespace MyPthreadImpl