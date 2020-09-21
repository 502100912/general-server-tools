#include "ut_framework.h"
#include <thread>
#include "blocking_map.h"

using namespace gserver;

void g_take(BlockingMap<int, std::string>* b_map, int key) {
    std::string* output = new std::string;
    b_map->take(key, output, 2000);
}

G_TEST(test_blocking_map_basic) {
    BlockingMap<int, std::string> b_map;
    //test init, init buckets num is 64
    int init_num = 60;
    int ret = b_map.init(init_num);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(b_map.buckets_num(), 64);

    //test put data[s1] with key[23] and take success
    std::string s1("HelloWorld");
    EXPECT_EQ(b_map.put(23, s1), 0);
    EXPECT_NE(b_map.put(23, s1), 0);
    EXPECT_EQ(b_map.size(), 1);
    std::string* output = new std::string;
    EXPECT_EQ(b_map.take(23, output, 2000), 0);
    EXPECT_EQ(output->compare("HelloWorld"), 0);

    //test take first
    std::thread t1(g_take, &b_map, 23);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread t2(g_take, &b_map, 87);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    b_map.put(87, "xiaohong");
    b_map.put(23, "xiaoming");
    t1.join();
    t2.join();
}

std::string g_str = "Some big data produce by producer";
void producer(BlockingMap<int, std::string>* b_map, int id, int batch) {
    printf("producer %d start!\n", id);
    int base = id * batch;
    for (int i = base; i < base + batch; i++) {
        std::string s = g_str + std::to_string(id) + "_" + std::to_string(i);
        b_map->put(i,s);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void consumer(BlockingMap<int, std::string>* b_map, int id, int batch) {
    printf("consumer %d start!\n", id);
    int base = id * batch;
    for (int i = base; i < base + batch; i++) {
        std::string* output = new std::string;
        if (i % 2 == 0) {
            b_map->take(i, output, 2000);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

G_TEST(test_blocking_gc) {
    BlockingMap<int, std::string> b_map;
    int ret = b_map.init(1024);
    EXPECT_EQ(b_map.buckets_num(), 1024);
    printf("Manager size %d\n", BlockingMapManager::instance()->get_objs().size());
    BlockingMapManager::instance()->reload(1000, 2000);
    std::vector<std::thread*> thread_manager;
    //4个生产者每个的生产速度qps1024
    for (int i = 1; i <= 4; i++) {
        std::thread* t = new std::thread(producer, &b_map, i, 10240);
        thread_manager.push_back(t);
    }
    //持续10s, 每秒1024 * 4, 共生产1024 * 4 * 10个string

    //4个消费者, 每秒消费512 * 4
    for (int i = 1; i <= 4; i++) {
        std::thread* t = new std::thread(consumer, &b_map, i, 10240);
        thread_manager.push_back(t);
    }

    //在启动gc线程时，每100ms检查下map size
    for(int i = 0; i < 120; i++) {
        printf("map size %d\n", b_map.size()); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (auto t_ptr: thread_manager) {
        t_ptr->join();
    }
    printf("map final size %d\n", b_map.size()); 
    std::cout << "Join All Thread" << std::endl;

}

int main() {
    G_RUNALL();
    return 0;
}
