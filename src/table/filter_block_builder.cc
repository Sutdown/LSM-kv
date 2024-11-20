#include <cassert>
#include "filter_block_builder.h"
#include "../src/filter/bloom_filter.h"
#include "../src/filter/filter_policy.h"
#include "../utils/codec.h"

namespace lsmkv
{
    FilterBlockBuilder::FilterBlockBuilder(int32_t keys_num, double false_positive = 0.01)
    {
        assert(false_positive >= 0.0);
        filterPolicy = std::make_shared<lsmkv::BloomFilter>(keys_num, false_positive);
    }

    void FilterBlockBuilder::add(const std::string &key)
    {
        if (key.empty())
            return;
        assert(filterPolicy != nullptr);
        _key_data.emplace_back(key);
    }

    void FilterBlockBuilder::finish_filter_block()
    {
        assert(filterPolicy != nullptr);
        create_filter();
        _data = filterPolicy->data();
        utils::PutFixed32(_data, filterPolicy->get_hash_num());
    }

    void FilterBlockBuilder::create_filter()
    {
        if (_key_data.empty())
        {
            return;
        }
        assert(filterPolicy != nullptr);
        filterPolicy->create_filter(_key_data);
    }

    bool FilterBlockBuilder::exists(const std::string_view &key)
    {
        assert(filterPolicy != nullptr);
        return filterPolicy->exists(key);
    }

}