#ifndef SKIPLIST_CONFIG_H
#define SKIPLIST_CONFIG_H

namespace lsmkv
{
    struct SkipListConfig
    {
        SkipListConfig() = delete;
        static constexpr int KMaxHeight = 12;
    };
}

#endif