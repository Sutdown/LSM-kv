#include <cstdint>

#ifdef NODE_H
#define NODE_H

namespace lsmkv
{
    template <typename K, typename V>
    class Node
    {
    public:
        Node() = default;

        K key;
        V *val; // 只进行深拷贝

        int32_t ref_cnt = 0;
        uint32_t hash_val = 0;

        bool in_cache = false;
        uint64_t last_access_time = 0;
    };
}

#endif