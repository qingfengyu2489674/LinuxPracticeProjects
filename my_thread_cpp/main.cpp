#include <iostream>
#include <vector>
#include <unistd.h>    // For sleep/usleep
#include <sys/syscall.h> // For SYS_gettid
#include <system_error> // For std::system_error if create throws
#include <stdexcept>    // For std::bad_alloc if create throws

#include "my_condition.h"
#include "my_thread.h"

// Global variables
MyPthreadImpl::MyMutex global_test_mutex;
MyPthreadImpl::MyCondition global_test_cv;

bool data_ready_flag = false;
int items_produced = 0;
int items_consumed = 0;
const int TOTAL_ITEMS_TO_TEST = 3;

// Producer thread function (C-style for my_pthread_create)
void* producer_func_c_style(void* arg) {
    (void)arg; // Unused argument for this simple example
    pid_t tid = syscall(SYS_gettid);
    std::cout << "Producer thread (Kernel TID: " << tid << ") started." << std::endl;

    for (int i = 0; i < TOTAL_ITEMS_TO_TEST; ++i) {
        usleep(150 * 1000); // Simulate work (150 milliseconds)

        global_test_mutex.lock();
        data_ready_flag = true;
        items_produced++;
        std::cout << "Producer (TID: " << tid << "): Produced item " << items_produced
                  << ". Notifying consumer." << std::endl;
        global_test_mutex.unlock();

        global_test_cv.notify_one();
    }

    usleep(100 * 1000); // Simulate delay before signaling end
    global_test_mutex.lock();
    data_ready_flag = true; // Set to true so consumer can see the -1
    items_produced = -1;    // Signal end of production
    std::cout << "Producer (TID: " << tid << "): Signaling end of production." << std::endl;
    global_test_mutex.unlock();
    global_test_cv.notify_one();

    std::cout << "Producer thread (Kernel TID: " << tid << ") finished." << std::endl;
    return nullptr; // C-style thread functions return void*
}

// Consumer thread function (C-style for my_pthread_create)
void* consumer_func_c_style(void* arg) {
    (void)arg; // Unused argument
    pid_t tid = syscall(SYS_gettid);
    std::cout << "Consumer thread (Kernel TID: " << tid << ") started." << std::endl;

    while (true) {
        global_test_mutex.lock();
        std::cout << "Consumer (TID: " << tid << "): Waiting for item... (data_ready=" << data_ready_flag
                  << ", items_produced=" << items_produced << ")" << std::endl;

        while (!data_ready_flag) {
            // Note: MyCondition::wait expects a MyMutex&, which global_test_mutex is.
            global_test_cv.wait(global_test_mutex);
            std::cout << "Consumer (TID: " << tid << "): Woken up. (data_ready=" << data_ready_flag
                      << ", items_produced=" << items_produced << ")" << std::endl;
        }

        if (items_produced == -1) { // Producer signaled end
            std::cout << "Consumer (TID: " << tid << "): Detected end signal from producer." << std::endl;
            global_test_mutex.unlock();
            break; // Exit the loop
        }

        // data_ready_flag is true here
        items_consumed++;
        std::cout << "Consumer (TID: " << tid << "): Consumed item. Total consumed: " << items_consumed << std::endl;
        data_ready_flag = false; // Reset for the next item

        // This check might be redundant if -1 signal is reliable, but good for robustness
        if (items_consumed >= TOTAL_ITEMS_TO_TEST && items_produced != -1) {
             std::cout << "Consumer (TID: " << tid <<"): Consumed all expected items based on count. Exiting." << std::endl;
            global_test_mutex.unlock();
            break;
        }
        global_test_mutex.unlock();
    }
    std::cout << "Consumer thread (Kernel TID: " << tid << ") finished. Total items consumed: " << items_consumed << std::endl;
    return nullptr; // C-style thread functions return void*
}

int main() {
    std::cout << "Main Thread (PID: " << getpid()
              << ", Kernel TID: " << syscall(SYS_gettid)
              << "): Starting Condition Variable Test with MyThread library..." << std::endl;

    MyPthreadImpl::MyThread* producer_tcb = nullptr;
    MyPthreadImpl::MyThread* consumer_tcb = nullptr;

    // Create producer thread
    std::cout << "Main: Creating producer thread..." << std::endl;
    try {
        int presult = MyPthreadImpl::my_pthread_create(&producer_tcb, producer_func_c_style, nullptr);
        if (presult == 0 && producer_tcb) {
            std::cout << "Main: Producer thread created successfully. TCB: " << producer_tcb
                      << ", Kernel TID (from TCB): " << producer_tcb->tid.load() << std::endl;
        } else {
            std::cerr << "Main: Failed to create producer thread, error code: " << presult << std::endl;
            // Handle error: perhaps exit or don't create consumer
        }
    } catch (const std::system_error& e) {
        std::cerr << "Main: System error creating producer thread: " << e.what() << " (Code: " << e.code().value() << ")" << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Main: Memory allocation error (bad_alloc) creating producer thread: " << e.what() << std::endl;
    }

    // Create consumer thread
    std::cout << "Main: Creating consumer thread..." << std::endl;
    try {
        int cresult = MyPthreadImpl::my_pthread_create(&consumer_tcb, consumer_func_c_style, nullptr);
        if (cresult == 0 && consumer_tcb) {
            std::cout << "Main: Consumer thread created successfully. TCB: " << consumer_tcb
                      << ", Kernel TID (from TCB): " << consumer_tcb->tid.load() << std::endl;
        } else {
            std::cerr << "Main: Failed to create consumer thread, error code: " << cresult << std::endl;
            // Handle error
        }
    } catch (const std::system_error& e) {
        std::cerr << "Main: System error creating consumer thread: " << e.what() << " (Code: " << e.code().value() << ")" << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Main: Memory allocation error (bad_alloc) creating consumer thread: " << e.what() << std::endl;
    }


    // Join threads
    if (producer_tcb) {
        std::cout << "Main: Joining producer thread (TCB: " << producer_tcb << ")..." << std::endl;
        MyPthreadImpl::my_pthread_join(producer_tcb);
        std::cout << "Main: Producer thread joined. Return value (from TCB): "
                  << reinterpret_cast<long>(producer_tcb->return_value) << std::endl;
        delete producer_tcb; // Clean up TCB
        producer_tcb = nullptr;
    }

    if (consumer_tcb) {
        std::cout << "Main: Joining consumer thread (TCB: " << consumer_tcb << ")..." << std::endl;
        MyPthreadImpl::my_pthread_join(consumer_tcb);
        std::cout << "Main: Consumer thread joined. Return value (from TCB): "
                  << reinterpret_cast<long>(consumer_tcb->return_value) << std::endl;
        delete consumer_tcb; // Clean up TCB
        consumer_tcb = nullptr;
    }

    std::cout << "\nMain: Condition Variable Test Finished." << std::endl;
    if (items_produced == -1 && items_consumed == TOTAL_ITEMS_TO_TEST) {
        std::cout << "Test Result: PASSED" << std::endl;
    } else {
        std::cout << "Test Result: FAILED (items_produced=" << items_produced
                  << ", items_consumed=" << items_consumed
                  << ", expected_consumed=" << TOTAL_ITEMS_TO_TEST << ")" << std::endl;
    }

    std::cout << "Main Thread: Exiting." << std::endl;
    return 0;
}