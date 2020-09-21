#ifndef REGION_BASED_MEMPOOL_H 
#define REGION_BASED_MEMPOOL_H 

#include<stdio.h>
#include<blocking_queue.h>

namespace gserver {
//TODO 修改为无锁
struct GlobalBlockListOption { 
    static const size_t MAX_BLOCK_COUNT = 256;
}

template<typename T, size_t CAP = MAX_FREE_LIST_NODE_CNT>
class GlobalBlockList {

};

class GlobalRegion {

};

class Mempool {
public:
    template<typename T, typename... Args>
    T* create(size_t num, Args&& ...args) {
        T* ins = static_cast<T*>self_malloc(sizeof(T));
        if (std::is_trivially_destructible<T>::value) {
            new (ins) T(std::forward<Args>(args)...);
            add_deleter(ins, &)
        }
        return ins;
    }
    template<typename T, typename... Args>
    T* create(size_t num, Args&& ...args) {
        T* ins = static_cast<T*>self_malloc(num * sizeof(T));
        if (!std::is_trivially_destructible<T>::value) {
            return ins;
        } else {
            for (int i = 0; i < num; i++) {

            }
        }
    }
piravate:
    inline void* self_malloc(size_t size) {
        static char ZERO_MALLOC = 0;
        if (size == 0) {
            return &ZERO_MALLOC;
        }
        size = _align(size);
        if (size <= _free_size) {
            void* p = _free_cursor;
            _free_size -= size;
            _free_cursor += size;
            return p;
        }
        return malloc_from_region(size);
    }
    void* malloc_from_region(size_t size);
    inline void add_deleter(void* data, )void* data,
priavate:
    // char _data[MAX_BLOCK_SIZE];
    size_t _free_size;
    char* _free_cursor;
    GlobalRegion _region;
};

};
