#include <thread>
#include <stdio.h>
#include "blocking_queue.h"

gserver::BlockingQueue<int,3> bq;

void consume(int n) {
    printf("consume!\n");
    for (int i = 0; i < n; i++) {
       int a = bq.wait_and_pop();
       printf("pop %d", a);
    }
}
void blocking_queue_common1() {
    std::thread consumer_thread(consume, 10);
    for (int i = 0; i < 10; i++) {
       bq.put(i);
       printf("put %d", i);
    }
    consumer_thread.join();
}

int main() {
    blocking_queue_common1();
    return 0;
}
