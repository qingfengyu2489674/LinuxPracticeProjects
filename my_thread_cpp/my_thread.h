#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <stdint.h>
#include <sys/types.h>
#include <atomic>
#include <cerrno>
#include <system_error>

#ifndef FUTEX_WAIT
#define FUTEX_WAIT 0
#endif
#ifndef FUTEX_WAIT_PRIVATE
#define FUTEX_WAIT_PRIVATE 128
#endif
#ifndef FUTEX_WAKE_PRIVATE
#define FUTEX_WAKE_PRIVATE 129
#endif

#ifndef __NR_munmap
    #ifdef __x86_64__
        #define __NR_munmap 11
    #else
        #define __NR_munmap 91
    #endif
#endif
#ifndef __NR_exit
    #ifdef __x86_64__
        #define __NR_exit 60
    #else
        #define __NR_exit 1
    #endif
#endif

namespace MyPthreadImpl {

constexpr size_t MY_DEFAULT_STACK_SIZE = (1024 * 1024);

class MyThread;

extern "C" int thread_entry_wrapper(void* tcb_arg);

class MyThread {
public:
    using StartRoutine = void* (*)(void*);

    std::atomic<pid_t> tid;
    StartRoutine start_routine_fn;
    void* arg_param;
    void* stack_base;
    size_t stack_alloc_size;
    void* return_value;

    MyThread(StartRoutine routine, void* arg, void* stack, size_t stack_size)
        : tid(0),
          start_routine_fn(routine),
          arg_param(arg),
          stack_base(stack),
          stack_alloc_size(stack_size),
          return_value(nullptr) {}

    MyThread(const MyThread&) = delete;
    MyThread& operator=(const MyThread&) = delete;
    MyThread(MyThread&&) = delete;
    MyThread& operator=(MyThread&&) = delete;
    ~MyThread() = default;

    friend int my_pthread_create(MyThread** thread_out,
                                 StartRoutine start_routine_fn,
                                 void* arg_param,
                                 size_t stack_size);
    friend void my_pthread_join(MyThread* target_thread);
};

int my_pthread_create(MyThread** thread_out,
                      MyThread::StartRoutine start_routine_fn,
                      void* arg_param,
                      size_t stack_size = MY_DEFAULT_STACK_SIZE);

void my_pthread_join(MyThread* target_thread);

} // namespace MyPthreadImpl