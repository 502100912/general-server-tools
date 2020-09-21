#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional> //for std::function

namespace g_server {

struct Task {
    std::function<void(void)> function;
    int tid;
    Task() : function(NULL), tid(-1) {
    }
};

class ThreadPool {
public:
    ThreadPool();
    void initialize(thread_num n);
    /* 线程初始化的入口，在thread_entry不断地fetch_task*/
    static void thread_entry();
ptivate:
    std::deque<Task> _task_queue;
    std::vector<std::thread> workers;
};

}

#endif
