// my_mutex.cpp
#include "my_mutex.h"

namespace MyPthreadImpl {

MyMutex::MyMutex() : futex_state(0) {}

void MyMutex::lock() {
    futex_lock(futex_state);
}

void MyMutex::unlock() {
    futex_unlock(futex_state);
}

} // namespace MyPthreadImpl