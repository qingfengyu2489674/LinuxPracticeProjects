#include "my_thread.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/syscall.h>


extern "C" int MyPthreadImpl::thread_entry_wrapper(void* tcb_arg) {
    MyPthreadImpl::MyThread* tcb = static_cast<MyPthreadImpl::MyThread*>(tcb_arg);

    void* stack_to_free = nullptr;
    size_t stack_size_to_free = 0;

    if (tcb) {
        stack_to_free = tcb->stack_base;
        stack_size_to_free = tcb->stack_alloc_size;
        if (tcb->start_routine_fn) {
            tcb->return_value = tcb->start_routine_fn(tcb->arg_param);
        }
    }

    if (stack_to_free && stack_size_to_free > 0) {
        uintptr_t p_stack_to_free = (uintptr_t)stack_to_free;
        size_t val_stack_size_to_free = stack_size_to_free;

        asm volatile(
            "movq %1, %%rdi\n\t"
            "movq %2, %%rsi\n\t"
            "movq %0, %%rax\n\t"
            "syscall\n\t"
            "movq $0, %%rdi\n\t"
            "movq %3, %%rax\n\t"
            "syscall"
            :
            : "i"(__NR_munmap),
              "r"(p_stack_to_free),
              "r"(val_stack_size_to_free),
              "i"(__NR_exit)
            : "rax", "rdi", "rsi", "memory", "cc", "r11", "rcx"
        );

    } else {
        asm volatile(
            "movq $0, %%rdi\n\t"
            "movq %0, %%rax\n\t"
            "syscall"
            :
            : "i"(__NR_exit)
            : "rax", "rdi", "memory", "cc", "r11", "rcx"
        );
    }
    return 0;
}

namespace MyPthreadImpl {

int my_pthread_create(MyThread** thread_out,
                      MyThread::StartRoutine start_routine_fn,
                      void* arg_param,
                      size_t stack_size) {
    if (!thread_out || !start_routine_fn) {
        return -EINVAL;
    }
    if (stack_size == 0) {
        stack_size = MY_DEFAULT_STACK_SIZE;
    }

    void* stack_memory = mmap(nullptr, stack_size,
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS,
                              -1, 0);
    if (stack_memory == MAP_FAILED) {
        throw std::system_error(errno, std::generic_category(), "my_pthread_create: mmap failed");
    }

    MyThread* tcb = nullptr;
    try {
        tcb = new MyThread(start_routine_fn, arg_param, stack_memory, stack_size);
    } catch (const std::bad_alloc& e) {
        munmap(stack_memory, stack_size);
        throw;
    } catch (...) {
        munmap(stack_memory, stack_size);
        delete tcb;
        throw;
    }

    void* child_stack_pointer = static_cast<char*>(stack_memory) + stack_size;
    
    int clone_flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM | CLONE_SETTLS |
                      CLONE_THREAD | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;

    pid_t new_tid = clone(MyPthreadImpl::thread_entry_wrapper,
                          child_stack_pointer,
                          clone_flags,
                          static_cast<void*>(tcb),
                          &tcb->tid,
                          static_cast<void*>(tcb),
                          &tcb->tid);

    if (new_tid == -1) {
        int err_no = errno;
        munmap(stack_memory, stack_size);
        delete tcb;
        throw std::system_error(err_no, std::generic_category(), "my_pthread_create: clone failed");
    }

    *thread_out = tcb;
    return 0;
}

void my_pthread_join(MyThread* target_thread) {
    if (!target_thread) {
        return;
    }

    pid_t current_tid_val;
    while ((current_tid_val = target_thread->tid.load(std::memory_order_acquire)) != 0) {

        // DEBUG:为什么只能使用 FUTEX_WAIT
        // 子线程退出时，由 CLONE_CHILD_CLEARTID 标志触发的内核 futex_wake 操作是一个 public 的唤醒操作。
        // 使用 FUTEX_WAIT_PRIVATE 会导致父线程“听不到”内核的唤醒信号，从而造成 join 操作永久阻塞。
        long ret = syscall(SYS_futex,
                           &target_thread->tid,
                           // FUTEX_WAIT_PRIVATE,
                           FUTEX_WAIT,
                           current_tid_val,
                           nullptr,
                           nullptr,
                           nullptr,
                           0);

        if (ret == -1 && errno != EAGAIN && errno != EINTR) {
            // Consider throwing an exception for robustness in a C++ library.
        }
    }
}






} // namespace MyPthreadImpl