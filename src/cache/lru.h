// 淘汰最长时间没有被使用的代码
#include <list>
#include <unordered_map>
#include <cassert>

#include "cache_policy.h"
#include "../utils/lock.h"
#include "../log/log.h"

#ifndef LRU_H
#define LRU_H

namespace lsmkv
{
    template <typename K, typename V, typename LockType = NullLock>
    class LRU final : public CachePolicy<K, V>
    {
    private:
        const uint32_t cap = 0; // capacity

        std::list<Node<K, V> *> nodes; // 存放结点指针

        std::unordered_map<K, typename std::std::list<Node<K, V> *>::iterator> index; // 保存键到结点的映射
        /*
         * 用于跟踪当前正在被使用的缓存条目
         * 确保这些条目不会被替换。这有助于提高性能，因为活跃的条目通常是程序当前需要的数据。
         * 通过将活跃条目与不活跃条目分开管理，可以减少对整个缓存的锁竞争
         */
        std::unordered_map<K, Node<K, V> *> garbage_station;  // 待删除列表，从index利用LRU策略删除后记录在此处
        std::function<void(const K &key, V *val)> destructor; // 回调函数

        LockType locker;
        static constexpr std::hash<K> hash_fn{}; // 计算key的哈希

    private:
        /*
         * 为什么需要引用计数？实际上这里存在两种策略：
         * 第一种是一旦LRU满了，就直接删除node，不需要什么引用计数，
         * 但是这种方法性能较低，因为每次查找Get都是直接返回一个node的深拷贝；
         * 第二种是采用引用计数的方法，Get直接返回一个node指针，但是这样的话，
         * 如果LRU满了，就不能直接删除node（此时引用计数不一定为0，表示被上层使用），
         * 需要把它暂时移动到回收站（指针移动），等待其引用计数降为0，才可以删除。
         * （管理生命周期，支持并发访问，避免资源泄露）
         */
        void phantom_erase_node(Node<K, V> *node)
        {
            if (node)
            {
                node->in_cache = false;
                nodes.erase(index[node->key]);
                index.erase(node->key);
                garbage_station[node->key] = node; // 移动到待删除队列
                unref(node);
            }
        }

        void ref(Node<K, V> *node)
        {
            if (node)
            {
                ++node->ref_cnt;
            }
        }

        void unref(Node<K, V> *node)
        {
            if (node)
            {
                --node->ref_cnt;
                if (node->ref_cnt <= 0)
                {
                    destructor(node->key, node->val);
                    delete node;
                    node = nullptr;
                }
            }
        }

    public:
        explicit LRU(uint32_t cap) : cap(cap) {}

        ~LRU()
        {
            // todo: 此处调用phantom_erase_node会出现double free or corruption (out)，暂时不知道原因
            // todo: 不析构也行，此处LRU的析构表示程序结束，此时内存会被系统回收
            //            for (auto &node: nodes) {
            //                // node被标记为0后，在phantom_erase_node中的unref函数中会被清除
            //                node->ref_cnt = 0;
            //                phantom_erase_node(node);
            //            }
        }

        void insert(const K &key, V *val) override
        {
            ScopedLock<LockType> lock_guard(locker);

            auto new_node = new Node<K, V>();
            new_node->key = key;
            new_node->val = val;
            new_node->ref_cnt = 1; // 引用计数初始化为1
            new_node->hash_val = hash_fn(key);
            new_node->in_cache = true;
            new_node->last_access_time = time(0); // 使用当前时间作为last_access_time

            auto f = index.find(new_node->key);
            if (f == index.end())
            {
                nodes.push_front(new_node);
                index[new_node->key] = nodes.begin();
                if (nodes.size() > cap)
                {
                    auto old_node = nodes.back();
                    phantom_erase_node(old_node);
                }
            }
            else
            {
                nodes.splice(nodes.begin(), nodes, index[new_node->key]);
                index[new_node->key] = nodes.begin();
            }
        }

        void erase(const K &key) override
        {
            ScopedLock<LockType> lock_guard(locker);
            auto iter = index.find(key);
            if (iter != index.end())
            {
                phantom_erase_node(*(iter->second));
            }
        }

        Node<K, V> *get(const K &key) override
        {
            ScopedLock<LockType> lock_guard(locker);
            auto iter = index.find(key);
            if (iter == index.end())
            {
                return nullptr;
            }
            nodes.splice(nodes.begin(), nodes, iter->second);
            index[key] = nodes.begin();
            ref(*(iter->second));
            return *(iter->second);
        }

        void release(const K &key) override
        {
            ScopedLock<LockType> lock_guard(locker);
            unref(*(index[key]));
        }

        void register_clean_handle(std::function<void(const K &key, V *val)> _destructor) override
        {
            this->destructor = _destructor;
        }
    };
}
#endif