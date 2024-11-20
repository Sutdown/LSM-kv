#include <cstdint>
#ifndef LOG_FORMAT_H
#define LOG_FORMAT_H

namespace lsmkv
{
    // 禁止实例化的设计适用于简单的静态配置，且防止误操作
    struct WALConfig
    {
        WALConfig() = delete;
        ~WALConfig() = delete;

        static constexpr int32_t KHeaderSize = 8;
    };

}

#endif