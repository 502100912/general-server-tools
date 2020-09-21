#include "mempool.h"

Mempool::malloc_from_region(size_t size) {
    //TODO 对大内存分配进行优化
    while (true) {
        fugue::memory::BlockReference* block = _blocks->get();
        if (block == NULL) {
            return NULL;
        }

        uint32_t free_size = fugue::memory::Block::BLOCK_SIZE - block->offset;
        if (size <= free_size) {
            if (_block) {
                _block->offset = fugue::memory::Block::BLOCK_SIZE - _free_size;
            }

            char* p = block->block->data + block->offset;
            _free_size = free_size - size;
            _free_cursor = p + size;
            _block = block;
            return p;
        }
    }
    return _blocks->malloc(size);
}
