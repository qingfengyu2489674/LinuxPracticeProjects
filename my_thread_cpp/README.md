
LinuxPracticeProjects - Simple Thread Library

这是一个基于 Linux futex 和 clone 系统调用实现的简易用户态线程库。它旨在提供一个轻量级的并发编程基础，用于学习和理解 Linux 线程机制的底层原理。

✨ 特性

轻量级实现：直接通过 clone 系统调用创建线程，避免了传统 POSIX 线程库的额外开销。

基于 futex 同步：互斥锁（my_mutex）和条件变量（my_condition）的实现依赖于 futex（Fast Userspace Mutex）系统调用，提供了高效的用户态同步机制。

学习用途：适合作为深入理解 Linux 进程/线程管理、futex 原理和底层并发编程的实践项目。

🛠️ 项目结构
.
├── main.cpp                # 示例代码或测试用例
├── Makefile                # 项目构建脚本
├── my_condition.cpp        # 条件变量实现
├── my_condition.h          # 条件变量头文件
├── my_futex_utils.h        # futex 辅助函数/宏
├── my_mutex.cpp            # 互斥锁实现
├── my_mutex.h              # 互斥锁头文件
├── my_platform.h           # 平台相关定义（如系统调用封装）
├── my_thread.cpp           # 线程创建与管理实现（基于 clone）
└── my_thread.h             # 线程库头文件
