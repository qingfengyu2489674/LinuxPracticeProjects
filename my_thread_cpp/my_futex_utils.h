// my_futex_utils.h
#pragma once

#include "my_platform.h"

namespace MyPthreadImpl {

inline void futex_lock(std::atomic<int>& futex_addr) {
    int expected_unlocked = 0;
    if (futex_addr.compare_exchange_strong(expected_unlocked, 1, std::memory_order_acquire, std::memory_order_relaxed)) {
        return;
    }

    while (true) {
        int current_val = futex_addr.load(std::memory_order_relaxed);
        if (current_val == 0) {
            if (futex_addr.compare_exchange_strong(current_val, 2, std::memory_order_acquire, std::memory_order_relaxed)) {
                return;
            }
            continue;
        } else if (current_val == 1) {
            if (futex_addr.compare_exchange_strong(current_val, 2, std::memory_order_acquire, std::memory_order_relaxed)) {
            }
            continue;
        }
        
        long ret = ::syscall(SYS_futex, &futex_addr, FUTEX_WAIT_PRIVATE, 2, nullptr, nullptr, 0);
        if (ret == -1 && errno != EAGAIN && errno != EINTR) {
            throw std::system_error(errno, std::generic_category(), "MyPthreadImpl::futex_lock: futex_wait failed");
        }
    }
}

inline void futex_unlock(std::atomic<int>& futex_addr) {
    if (futex_addr.exchange(0, std::memory_order_release) == 2) {
        long ret = ::syscall(SYS_futex, &futex_addr, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
        if (ret == -1 && errno != EAGAIN && errno != EINTR) {
            // Error on wake is less critical, could be logged.
        }
    }
}

} // namespace MyPthreadImpl