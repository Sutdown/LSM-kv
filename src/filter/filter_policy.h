#include <string>
#include <string_view>
#include <vector>

#ifndef FILTER_POLICY_H
#define FILTER_POLICY_H

namespace lsmkv
{
    class FilterPolicy
    {
    public:
        FilterPolicy() = default;
        // 确保派生类正确清理资源
        virtual ~FilterPolicy() = default;

        virtual std::string &data() = 0;
        // 返回hash函数的数量
        virtual int32_t get_hash_num() = 0;

        // 返回过滤策略的名称，如果过滤器的编码方式发生不兼容的变化，该名称也需要更改
        // 否则旧的不兼容的过滤器被错误的传递到方法中
        virtual std::string policy_name() = 0;

        // 根据给定的键列表生成过滤器
        virtual void create_filter(const std::vector<std::string> &keys) = 0;

        // 看给定的键是否可能在过滤器中
        virtual bool exists(const std::string_view &key) = 0;

        virtual uint64_t size() = 0;
    };
}

#endif