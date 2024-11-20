#ifndef MEMTABLE_CONFIG_H
#define MEMTABLE_CONFIG_H

namespace lsmkv
{
    struct MemTableConfig
    {
        MemTableConfig() = delete;
        static constexpr int MAX_KEY_NUM = 4096;
    };

}

#endif