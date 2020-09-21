#include "ut_framework.h"
#include <thread>
#include <stdio.h>
#include "blocking_queue.h"
#include <random>
#include <string>

using namespace gserver;
std::default_random_engine e;
std::uniform_int_distribution<unsigned> u(0,100);

template<typename T, size_t MAXSIZE = 0>
void wait_and_pop_N(BlockingQueue<T, MAXSIZE>* bq, int n) {
    printf("wait_and_pop[%d]!\n", n);
    for (int i = 0; i < n; i++) {
       T t;
       if (bq->wait_and_pop(t)) {
           printf("wait succ, pop:[%d]\n", t);
       } else {
           printf("wait ret false\n");
       }
    }
}

template<typename T, size_t MAXSIZE = 0>
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

template <typename T, size_t MAXSIZE = 0>
void put_N(BlockingQueue<T, MAXSIZE>* bq, int n) {
    printf("put[%d]!\n", n);
    for (int i = 0; i < n; i++) {
        unsigned int ele = u(e);
        if (bq->put(u(e))) {
            printf("put succ:[%d]\n", ele);
        }
    }
}

template <typename T, size_t MAXSIZE = 0>
void put_N_and_release(BlockingQueue<T, MAXSIZE>* bq, int n) {
    printf("put_N_and_release[%d]!\n", n);
    for (int i = 0; i < n; i++) {
        unsigned int ele = u(e);
        if (bq->put(ele)) {
            printf("put succ:[%d]\n", ele);
        }
    }
    bq->release();
}

G_TEST(test_unlimited_storage_blocking_queue) {
    printf("test_unlimited_storage_blocking_queue_release\n");
    BlockingQueue<int> bq;
    std::thread thread1(wait_and_pop_N<int>, &bq, 7); //consume 7 objects with blocked interface
    std::thread thread2(put_N<int>, &bq, 10); //produce 10 objects, after 7 consume remain 3 objects
    thread2.join();
    thread1.join();
    std::thread thread3(wait_and_pop_N<int>, &bq, 2); //return 2 objects immediately
    thread3.join();
    std::thread thread4(try_pop_N<int>, &bq, 2);//succ 1 and failed 1
    thread4.join();
    //dont forget join all thread
}

G_TEST(test_unlimited_storage_blocking_queue_release) {
    printf("test_unlimited_storage_blocking_queue_release\n");
    BlockingQueue<int> bq;
    std::thread thread2(put_N_and_release<int>, &bq, 5); //produce 10 objects, after 7 consume remain 3 objects
    std::thread thread1(wait_and_pop_N<int>, &bq, 7); //consume 7 objects with blocked interface
    thread2.join();
    thread1.join();
}

G_TEST(test_bq_init_with_size) {
    printf("test_limited_storage_bq_init_with_size\n");
    BlockingQueue<int> bq(20, 3);
    //with MAX_SIZE 10, init 10 string
    BlockingQueue<int, 12> bq2(20, 4);
    std::thread thread1(wait_and_pop_N<int>, &bq, 7); //consume 7 objects with blocked interface
    printf("%d %d\n", bq.size(), bq2.size());
}


int main() {
    G_RUNALL();
    return 0;
}
