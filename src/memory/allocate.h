#include <cstdint>
#include <array>
#include <gtest/gtest.h>

#ifndef ALLOCATE_H
#define ALLOCATE_H

namespace lsmkv
{
    union BlockNode
    {
        BlockNode *next;  // 指向下一個塊的地址
        void *block_addr; // 當前數據塊存儲的地址
    };

    // 只为需要小于4kb的内存块分配内存
    // 内存池：一大块预分配的内存区域
    // 内存槽 + 内存块：对内存池进行组织和分类内存池中大小不同内存块的工具；
    // 内存槽的核心在于记录和组织内存池中的空闲内存块
    class FreeListAllocate
    {
    public:
        FreeListAllocate();
        ~FreeListAllocate() = default;

        void *Allocate(int32_t n);
        void *Deallocate(void *p, int32_t);
        void *Reallocate(void *p, int32_t old_size, int new_size);

    private:
        void fill_slot(int32_t slot_index); // 填充内存槽
        void fill_mem_pool();               // 申请一块内存作为内存池
        static inline int32_t get_index(int32_t);

    private:
        constexpr static int32_t SLOT_NUM = 512;
        constexpr static int32_t ALIGN_BYTES = 8;
        constexpr static int32_t MAX_OBJ_SIZE = 4 * 1024;
        constexpr static int32_t CHUNK_SIZE = 4 * 1024 * 1024;
        constexpr static int32_t FILL_BLOCK_CNT = 10;

    private:
        std::array<BlockNode *, SLOT_NUM> memory_slot;
        char *mem_pool_start = nullptr;
        int32_t mem_pool_size = 0;
    };
}

#endif
