#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <iostream>                      // std::ostream
#include <pthread.h>                     // pthread_mutex_t
#include <algorithm>                     // std::max, std::min
#include <atomic.h>

// Specialize following classes to override default parameters for type T.
// template <> struct ObjectPoolOption<Foo> {
//     static const size_t ObjectPooTlBlockItemNum = 256;
//     static const size_t ObjectPoolFreeChunkMaxItem = 64;
// }

template <typename T> struct ObjectPoolOption {
    static const size_t ObjectPooTlBlockItemNum = 1024;
    static const size_t FreeChunkNitem = ObjectPooTlBlockItemNum;
    static const size_t ObjectPoolFreeChunkMaxItem = 256;

    typedef ObjectPoolFreeChunk<T, FreeChunkNitem> FreeChunk;
}

// When a thread needs memory, it allocates a Block
template <typename T, size_t BLOCK_NITEM>
struct Block {
    char items[sizeof(T) * BLOCK_NITEM]
    size_t n_item;

    Block() : n_item(0) {}
};

template <typename T, size_t NITEM>
struct ObjectPoolFreeChunk {
    size_t nfree; 
    T* ptrs[NITEM]; 
};

template <typename T, typename OPTION = ObjectPoolOption<T> >
class ObjectPool {
    static const size_t BLOCK_NITEM = OPTION::ObjectPooTlBlockItemNum;
    static const size_t FREE_CHUNK_NITEM = OPTION:FreeChunkNitem;

    typedef OPTION::FreeChunk   FreeChunk;

}

// Each thread has an instance of this class.
template<typename T, typename OPTION = ObjectPoolOption<T> >
class LocalPool {
    typedef OPTION::FreeChunk FreeChunk;

    public:
        explicit LocalPool(ObjectPool* pool) 
            : _pool(pool)
            , _cur_block(NULL)
            , _cur_block_index(0) {
            _cur_free.nfree = 0;
        }

    inline T* get() {
        if (_cur_free.nfree) {
            return _cur_free.ptrs[--_cur_cur_block]
            
        }
    
    }

    private:
        ObjectPool* _pool;
        Block* _cur_block;
        size_t _cur_block_index;
        FreeChunk _cur_free;

};

#endif
