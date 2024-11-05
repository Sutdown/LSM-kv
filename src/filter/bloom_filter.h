#include <cstdint>
#include <string>
#include <vector>
#include "filter_policy.h"

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

namespace lsmkv
{
    class BloomFilter final : public FilterPolicy
    {
    public:
        BloomFilter() = delete;
        /*explicit BloomFilterPolicy(int bits_per_key) : bits_per_key_(bits_per_key)*/
        BloomFilter(int32_t keys_num, double false_positive);
        ~BloomFilter() override = default;

        std::string policy_name() override;

        uint64_t size() override;

        void create_filter(const std::vector<std::string> &keys) override;

        bool exists(const std::string_view &key) override;

    private:
        int32_t hash_func_num = 0; // 哈希函数个数
        int32_t bits_per_key = 0;  // 每个键的比特数
        std::vector<uint8_t> bits_array;
    };
}

#endif