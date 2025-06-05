// my_condition.cpp
#include "my_condition.h"
#include "my_platform.h"

namespace MyPthreadImpl {

MyCondition::MyCondition() : cond_sequence(0) {}

void MyCondition::wait(MyMutex& external_mutex) {
    unsigned int current_seq_val = cond_sequence.load(std::memory_order_acquire);

    while (current_seq_val & 1) {
        unsigned int expected_val = current_seq_val;
        if (cond_sequence.compare_exchange_weak(expected_val, current_seq_val + 1,
                                               std::memory_order_acq_rel, 
                                               std::memory_order_acquire)) {
            current_seq_val = current_seq_val + 1;
        } else {
            current_seq_val = expected_val;
        }
    }

    external_mutex.unlock();

    long ret = ::syscall(SYS_futex, &cond_sequence, FUTEX_WAIT_PRIVATE, current_seq_val, nullptr, nullptr, 0);
    
    bool relock_failed = false;
    try {
        external_mutex.lock();
    } catch (...) {
        relock_failed = true;
    }

    if (ret == -1 && errno != EAGAIN && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "MyCondition::wait: FUTEX_WAIT_PRIVATE failed");
    }
    if (relock_failed) {
        throw; 
    }
}

void MyCondition::notify_one() {
    cond_sequence.fetch_or(1, std::memory_order_release);
    long ret = ::syscall(SYS_futex, &cond_sequence, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
    if (ret == -1 && errno != EAGAIN && errno != EINTR) {
        // Error handling for wake (e.g., logging) can be added here if needed
    }
}


} // namespace MyPthreadImpl