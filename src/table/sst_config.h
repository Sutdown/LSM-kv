#include <cstdint>

#ifndef SST_CONFIG_H
#define SST_CONFIG_H

namespace lsmkv
{
    struct SSTConfigInfo
    {
        SSTConfigInfo() = delete;
        ~SSTConfigInfo() = delete;

        // 设置重启点，查找时直接从最近的重启点开始比较
        static constexpr int32_t RESTART_INTERVAL = 16;
        // data block中的最大大小
        static constexpr int32_t MAX_DATA_BLOCK_SIZE = 4096;
    };
}

#endif