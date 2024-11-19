#include <cassert>
#include "allocate.h"
#include "../log/log.h"

namespace lsmkv
{
    auto logger = log::log::get_instance();
    FreeListAllocate::FreeListAllocate()
    {
        memory_slot.fill(nullptr); // 内存槽初始化

        mem_pool_start = (char *)malloc(CHUNK_SIZE);
        mem_pool_size = CHUNK_SIZE;
    }

    void *FreeListAllocate::Allocate(int32_t n)
    {
        // 异常情况和malloc
        if (n <= 0)
        {
            logger->error("The n is negative or zero. n = {}", n);
            return nullptr;
        }
        if (n > MAX_OBJ_SIZE)
        {
            return malloc(n);
        }

        // 正常分配
        auto slot_index = get_index(n);
        if (memory_slot[slot_index] == nullptr)
        {
            fill_slot(slot_index);
        }

        // 内存槽始终指向空闲块的第一个位置
        BlockNode *ret = memory_slot[slot_index];
        memory_slot[slot_index] = ret->next;
        return ret;
    }

    void FreeListAllocate::Deallocate(void *p, int32_t n)
    {
        if (p == nullptr || n == 0)
        {
            logger->error("p cannot be nullptr, n cannot be zero. p={}, n={}", p, n);
            return;
        }
        if (n > MAX_OBJ_SIZE)
        {
            free(p);
            p = nullptr;
            return;
        }

        // 将释放的内存块收回槽中，头插法
        auto slot_index = get_index(n);
        auto node = static_cast<BlockNode *>(p);
        node->next = memory_slot[slot_index];
        memory_slot[slot_index] = node;
    }

    void *FreeListAllocate::Reallocate(void *p, int32_t old_size, int new_size)
    {
        Deallocate(p, old_size);
        return Allocate(new_size);
    }

    // 填满序号slot_index的内存槽，内存槽下的对应需要会存放分配的10个内存块
    // 保证这块在管理之中，便于分配
    void FreeListAllocate::fill_slot(int32_t slot_index)
    {
        // 一次性申请10个block，挂载到memory_slot上
        int32_t block_size = (slot_index + 1) * 8; // 当前内存槽的block大小
        int32_t needed_size = FILL_BLOCK_CNT * block_size;

        // 分配空间
        if (mem_pool_size >= needed_size)
        {
            // 内存池大小完全满足需要
            for (int i = 0; i < FILL_BLOCK_CNT; i++)
            {
                auto node = reinterpret_cast<BlockNode *>(mem_pool_start + i * block_size);
                if (i == 0)
                {
                    node->next == nullptr;
                }
                // 头插法
                node->next = memory_slot[slot_index];
                memory_slot[slot_index] = node;
            }
            mem_pool_start += needed_size;
            mem_pool_size -= needed_size;
        }
        else if (mem_pool_size >= block_size)
        {
            // 内存池至少满足一个以上block的需要
            int32_t cnt = mem_pool_size / block_size;
            for (int i = 0; i < cnt; i++)
            {
                auto node = reinterpret_cast<BlockNode *>(mem_pool_start + i * block_size);
                if (i == 0)
                {
                    node->next == nullptr;
                }
                // 头插法
                node->next = memory_slot[slot_index];
                memory_slot[slot_index] = node;
            }
            mem_pool_start += cnt * block_size;
            mem_pool_size -= cnt * block_size;
        }
        else
        {
            // 内存池一个block都无法满足
            assert(mem_pool_size % 8 == 0);

            // 将剩余部分挂载到slot上面，防止浪费
            if (mem_pool_size >= 8)
            {
                int32_t target_slot_index = get_index(mem_pool_size);
                auto node = reinterpret_cast<BlockNode *>(mem_pool_start);
                node->next = memory_slot[target_slot_index];
                mem_pool_start = nullptr;
                mem_pool_size = 0;
            }

            // 重新申请一块内存池
            logger->debug("func fill_mem_pool is called.");
            fill_mem_pool();

            // 重新执行本函数
            // 请求的内存没有分配完时会继续分配
            fill_slot(slot_index);
        }
    }

    // 分配一个新的4MB的内存池，旧的内存池会自行生命周期结束后析构释放资源
    void FreeListAllocate::fill_mem_pool()
    {
        mem_pool_start = (char *)malloc(CHUNK_SIZE);
        if (mem_pool_start == nullptr)
        {
            logger->error("Memory allocation failed.");
            return;
        }
        logger->debug("{}MB memory is allocated once", CHUNK_SIZE);
        mem_pool_size = CHUNK_SIZE;
    }

    int FreeListAllocate::get_index(int32_t n)
    {
        return static_cast<int>(n + 7) / 8 - 1;
    }
}
