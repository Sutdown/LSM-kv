#include "murmur_hash2.h"

namespace lsm::utils
{
    inline uint32_t murmur_hash2(const void *key, uint32_t len)
    {
        static constexpr uint32_t seed = 0xbc451d34;
        static constexpr uint32_t m = 0x5bd1e995;
        static constexpr uint32_t r = 24;

        uint32_t h = seed ^ len;

        // 将输入数据分成每块4字节处理，每块进行乘法，右移，异或等操作
        const uint8_t *data = (const unsigned char *)key;

        while (len >= 4)
        {
            // 将 4 字节的数据（32 位）转换为 uint32_t 型
            uint32_t k = *(uint32_t *)data;

            // MurmurHash2 的核心混合操作
            k *= m;
            k ^= k >> r;
            k *= m;
            h *= m;
            h ^= k;

            data += 4;
            len -= 4;
        }

        // 不是4的倍数，剩下字节单独处理
        switch (len)
        {
        case 3:
            h ^= data[2] << 16;
        case 2:
            h ^= data[1] << 8;
        case 1:
            h ^= data[0];
            h *= m;
        };

        // 哈希结束后，通过右移和异或进一步打散哈希值
        h ^= h >> 13;
        h *= m;
        h ^= h >> 15;

        // 返回哈希值
        return h;
    }
}