#include <vector>
#include <cstdint>
#include <string>
#include <memory>

#include "cache_policy.h"
#include "lru.h"

#ifndef CACHE_H
#define CACHE_H

namespace lsmkv
{
    // 实现分片的LRU缓存
    template <typename K, typename V>
    class Cache
    {
    private:
        // 设置分片，也就是多个个LRU Holder。一定程度上可以减少碰撞
        // 此外分片还可以减少锁的粒度(将锁的范围减少到原来的1/SHARDING_NUM)，提高了并发性
        static constexpr uint64_t SHARDING_NUM = 5;
        std::vector<std::shared_ptr<CachePolicy<K, V>>> caches;

        static constexpr std::hash<K> hash_fn{};

    public:
        explicit Cache(uint32_t cap)
        {
            caches.resize(SHARDING_NUM);
            for (int i = 0; i < SHARDING_NUM; ++i)
            {
                caches[i] = std::make_shared<LRU<K, V, MutexLock>>(cap);
            }
        }

        ~Cache() = default;

        void insert(const K &key, V *val)
        {
            uint64_t sharding_index = hash_fn(key) % SHARDING_NUM;
            caches[sharding_index]->insert(key, val);
        }

        void erase(const K &key)
        {
            uint64_t sharding_index = hash_fn(key) % SHARDING_NUM;
            caches[sharding_index]->erase(key);
        }

        Node<K, V> *get(const K &key)
        {
            uint64_t sharding_index = hash_fn(key) % SHARDING_NUM;
            return caches[sharding_index]->get(key);
        }

        void release(const K &key)
        {
            uint64_t sharding_index = hash_fn(key) % SHARDING_NUM;
            caches[sharding_index]->release(key);
        }

        /*
        通过 register_clean_handle 函数，
        将一个统一的清理回调函数destructor注册到所有的缓存分片（caches[i]）
        */
        void register_clean_handle(std::function<void(const K &key, V *val)> destructor)
        {
            for (int i = 0; i < SHARDING_NUM; ++i)
            {
                caches[i]->register_clean_handle(destructor);
            }
        }
    };
}

#endif