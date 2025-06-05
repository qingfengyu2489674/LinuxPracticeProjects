// my_platform.h
#pragma once

#include <atomic>
#include <system_error>
#include <cerrno>
#include <unistd.h> 

#include <sys/syscall.h> 
#include <linux/futex.h> 
#include <sys/mman.h>    

#ifndef FUTEX_WAIT_PRIVATE
#define FUTEX_WAIT_PRIVATE 128
#endif

#ifndef FUTEX_WAKE_PRIVATE
#define FUTEX_WAKE_PRIVATE 129
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef SYS_futex
    #if defined(__x86_64__)
        #define SYS_futex 202
    #elif defined(__i386__)
        #define SYS_futex 240
    #elif defined(__aarch64__)
        #define SYS_futex 98
    #elif defined(__arm__)
        #define SYS_futex 240 
    #else
        #error "SYS_futex is not defined for this architecture in syscall.h and no fallback provided."
    #endif
#endif

#ifndef SYS_brk 
    #if defined(__x86_64__)
        #define SYS_brk 12
    #elif defined(__i386__)
        #define SYS_brk 45
    #elif defined(__aarch64__)
        #define SYS_brk 214
    #elif defined(__arm__)
        #define SYS_brk 45
    #else
        #error "SYS_brk is not defined for this architecture in syscall.h and no fallback provided."
    #endif
#endif