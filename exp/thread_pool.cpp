#include <thread_pool.h>
#include <stdio.h>
#include <time.h>

void ThreadPool:initialize(thread_num n) {
    for (size_t i = 0; i <= n; i++) {
        workers.emplace_back(std::thread(ThreadPool::thread_entry));   
    }
}

void ThreadPool:thread_entry() {
    // fetch task and do task
    printf("time is:%d\n", time.time())
}
