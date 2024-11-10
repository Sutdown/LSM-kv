#include <cstdint>
#include "node.h"
#include <functional>

#ifndef CACHE_POLICY_H
#define CACHE_POLICY_H

namespace lsmkv
{
    template <typename K, typename V>
    class CachePolicy
    {
    public:
        CachePolicy() = default;
        virtual ~CachePolicy() = default;

        virtual void insert(const K &key, V *val) = 0;
        virtual void erase(const K &key) = 0;
        virtual Node<K, V> *get(const K &key) = 0;

        virtual void release(const K &key) = 0;
        virtual void register_clean_handle(std::function<void(const K &key, V *val)> destructor) = 0;
    };
}

#endif