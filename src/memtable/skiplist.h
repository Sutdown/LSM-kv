#include <memory>
#include <vector>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <optional>
#include "../log/log.h"
#include "../memory/allocate.h"
#include "skiplist_config.h"

#ifndef SKIPLIST_H
#define SKIPLIST_H

namespace lsmkv
{
    // 粗略的实现，缺乏线程安全，不支持重复的key
    template <typename Key, typename Value>
    class SkipList
    {
        class Node;

    public:
        explicit SkipList(std::shared_ptr<FreeListAllocate> alloc);

        void Insert(const Key &key, const Value &value);
        void Delete(const Key &key);
        bool Contains(const Key &key);
        std::optional<Value> Get(const Key &key);

        inline int GetSize() { return size; }

    private:
        int RandomLevel();
        int GetCurrentHeight();
        // 找到前缀节点
        void FindPrevNode(const Key &key, std::vector<Node *> &prev);
        inline Node *NewNode(const Key &key, int level, const Value &value);

    private:
        Node *head_; // 最高层头结点
        std::shared_ptr<FreeListAllocate> alloc;

        int max_level;
        int64_t size = 0;

        std::shared_ptr<spdlog::logger> logger = log::get_logger();
    };

    template <typename Key, typename Value>
    SkipList<Key, Value>::SkipList(std::shared_ptr<FreeListAllocate> alloc)
    {
        srand(time(0));
        head_ = NewNode("", SkipListConfig::KMaxHeight, "");
        max_level = 1;
        size = 0;
    }

    // public
    template <typename Key, typename Value>
    void SkipList<Key, Value>::Insert(const Key &key, const Value &value)
    {
        if (Contains(key))
        {
            logger->warn("A duplicate key was inserted. Key={}", key);
            return;
        }

        ++size;
        // todo：可优化为 std::vector<Node *> prev(GetCurrentHeight, nullptr);
        std::vector<Node *> prev(SkipListConfig::kMaxHeight, nullptr);

        FindPrevNode(key, prev);
        int level_of_new_node = RandomLevel();
        max_level = std::max(level_of_new_node, max_level);
        auto newNode = NewNode(key, level_of_new_node, value);

        for (int i = 0; i < newNode->GetLevel(); ++i)
        {
            if (prev[i] == nullptr)
            {
                newNode->next[i] = nullptr;
                head_->next[i] = newNode;
            }
            else
            {
                newNode->next[i] = prev[i]->next[i];
                prev[i]->next[i] = newNode;
            }
        }
    }

    template <typename Key, typename Value>
    void SkipList<Key, Value>::Delete(const Key &key)
    {
        if (Contains(key) == false)
        {
            logger->warn("The value you want to delete does not exist. Key={}", key);
            return;
        }
        --size;
        std::vector<Node *> prev(SkipListConfig::kMaxHeight, nullptr);

        int level = GetCurrentHeight() - 1;
        auto cur = head_;
        int level_of_target_node = -1; // 目标节点的层数
        while (true)
        {
            auto next = cur->next[level];
            if (next == nullptr)
            {
                if (level == 0)
                {
                    logger->error("A error point.");
                    break; // 遍历完成. 实际上这个分支不可能到达
                }
                else
                    // prev[level] = cur;
                    --level;
            }
            else
            {
                if (next->key == key)
                {
                    level_of_target_node = next->GetLevel();
                    prev[level] = cur;
                    --level;
                    if (level < 0)
                        break;
                }
                else if (next->key < key)
                    cur = next;
                else if (next->key > key)
                {
                    prev[level] = cur;
                    --level;
                    if (level < 0)
                        break;
                }
            }
        }

        for (int i = 0; i < level_of_target_node; ++i)
        {
            if (prev[i] != nullptr)
            {
                assert(prev[i]->next[i] != nullptr);
                prev[i]->next[i] = prev[i]->next[i]->next[i];
            }
        }
    }

    template <typename Key, typename Value>
    bool SkipList<Key, Value>::Contains(const Key &key)
    {
        int level = GetCurrentHeight() - 1;
        auto cur = head_;
        while (true)
        {
            auto next = cur->next[level];
            if (next == nullptr)
            {
                if (level == 0)
                {
                    return false;
                }
                else
                {
                    --level;
                }
            }
            else
            {
                if (next->key == key)
                {
                    return true;
                }
                else if (next->key < key)
                {
                    cur = next;
                }
                else if (next->key > key)
                {
                    if (level == 0)
                    {
                        return false; // 只有在最后一层，遇到大于key的时候才可以认为没找到
                    }
                    else
                    {
                        --level; // 在非最底层遇到了大于key的数，应该下降
                    }
                }
            }
        }
    }

    template <typename Key, typename Value>
    std::optional<Value> Get(const Key &key)
    {
        int level = GetCurrentHeight() - 1;
        auto cur = head_;
        while (true)
        {
            auto next = cur->next[level];
            if (next == nullptr)
            {
                if (level == 0)
                    // 遍历到这里说明key不存在
                    return std::nullopt;
                else
                    --level;
            }
            else
            {
                if (next->key == key)
                    return next->value; // 找到了
                else if (next->key < key)
                    cur = next;
                else if (next->key > key)
                    if (level == 0)
                        // 遍历到这里说明key不存在
                        return std::nullopt;
                    else
                        --level; // 在非最底层遇到了大于key的数，应该下降
            }
        }
    }

    // private
    template <typename Key, typename Value>
    int SkipList<Key, Value>::RandomLevel()
    {
        int level = 1;
        while (level < SkipListConfig::KMaxHeight && rand() % 1)
        {
            ++level;
        }
        return level;
    }

    template <typename Key, typename Value>
    int SkipList<Key, Value>::GetCurrentHeight()
    {
        return max_level;
    }

    template <typename Key, typename Value>
    void SkipList<Key, Value>::FindPrevNode(const Key &key, std::vector<Node *> &prev)
    {
        int level = GetCurrentHeight() - 1;
        auto cur = head_; // 最高层的头结点
        while (true)
        {
            auto next_node = cur->next[level];
            if (next_node == nullptr || next_node->key >= key)
            {
                // 继续向下一层寻找
                prev[level] = cur;
                if (level > 0)
                {
                    --level;
                }
                else
                {
                    return;
                }
            }
            else
            {
                cur = next_node;
            }
        }
    }

    template <typename Key, typename Value>
    inline SkipList<Key, Value>::Node *SkipList<Key, Value>::NewNode(const Key &key, int level, const Value &value)
    {
        // 有一定风险
        return new Node(key, level, value);
    }

    template <typename Key, typename Value>
    class SkipList<Key, Value>::Node
    {
    public:
        Node() = delete;
        Node(const Key &key, int level, const Value &value) : key(key), value(value)
        {
            next.resize(level, nullptr);
        }
        ~Node() = default;

        inline int GetLevel()
        {
            return next.size();
        }

        const Key key;
        Value value;
        std::vector<Node *> next; // 该结点在每一层的后继节点
    };
}

#endif