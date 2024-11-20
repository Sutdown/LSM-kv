#include <string>
#include <memory>
#include <vector>

#ifndef FILTER_BLOCK_BUILDER_H
#define FILTER_BLOCK_BUILDER_H

namespace lsmkv
{
    class FilterPolicy; // 前向声明

    class FilterBlockBuilder final
    {
    private:
        std::shared_ptr<FilterPolicy> filterPolicy = nullptr;

        // 直接用数组保存，存在一定的内存开销（有待改进）
        std::vector<std::string> _key_data;

        std::string _data;

    public:
        explicit FilterBlockBuilder(int32_t keys_num, double false_positive);
        ~FilterBlockBuilder() = default;

        // 添加到_data中，并不会创建过滤器
        void add(const std::string &key);

        void finish_filter_block();

        inline std::string_view data() { return _data; }

        inline void clear()
        {
            _key_data.clear();
            _data.clear();
        }

        bool exists(const std::string_view &key);

    private:
        inline void create_filter();
    };
}

#endif