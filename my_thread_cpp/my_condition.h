// my_condition.h
#pragma once

#include "my_futex_utils.h"
#include "my_mutex.h" 

namespace MyPthreadImpl {

class MyCondition {
private:
    std::atomic<unsigned int> cond_sequence;

public:
    MyCondition();
    ~MyCondition() = default;

    MyCondition(const MyCondition&) = delete;
    MyCondition& operator=(const MyCondition&) = delete;

    void wait(MyMutex& external_mutex);

    void notify_one();
};

} // namespace MyPthreadImpl