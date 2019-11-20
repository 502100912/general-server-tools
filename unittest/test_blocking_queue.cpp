#include "ut_framework.h"
#include <thread>
#include <stdio.h>
#include "blocking_queue.h"
#include <random>

using namespace gserver;
std::default_random_engine e;
std::uniform_int_distribution<unsigned> u(0,100);

template<typename T, int MAXSIZE = 0>
void wait_and_pop_N(BlockingQueue<T, MAXSIZE>* bq, int n) {
    printf("wait_and_pop[%d]!\n", n);
    for (int i = 0; i < n; i++) {
       unsigned int a = bq->wait_and_pop();
       printf("wait succ, pop:[%d]\n", a);
    }
}

template<typename T, int MAXSIZE = 0>
void try_pop_N(BlockingQueue<T, MAXSIZE>* bq, int n) {
    printf("try_pop[%d]!\n", n);
    for (int i = 0; i < n; i++) {
        T ret; 
        bool succ =  bq->try_pop(ret);
        if (succ) {
            printf("try pop succ:[%d]\n", ret);
        } else {
            printf("try pop failed\n");
        }
    }
}

template <typename T, int MAXSIZE = 0>
void put_N(BlockingQueue<T, MAXSIZE>* bq, int n) {
    printf("put[%d]!\n", n);
    for (int i = 0; i < n; i++) {
        T ele = bq->put(u(e));
        printf("put succ:[%d]\n", ele);
    }
}

G_TEST(test_unlimited_storage_blocking_queue) {
    BlockingQueue<int> bq;
    std::thread thread1(wait_and_pop_N<int>, &bq, 7);
    std::thread thread2(put_N<int>, &bq, 10);
    thread2.join();
    std::thread thread3(wait_and_pop_N<int>, &bq, 2);
    std::thread thread4(try_pop_N<int>, &bq, 2);
    thread1.join();
    thread3.join();
    thread4.join();
    //dont forget join all thread
}


int main() {
    G_RUNALL();
    return 0;
}
