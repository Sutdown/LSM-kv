#include <iostream>
#include "utils/murmur_hash2.h"
#include "bloom_filter.h"
#include <cmath>

namespace lsmkv
{
    /*
    explicit BloomFilterPolicy(int bits_per_key) : bits_per_key_(bits_per_key) {
    // hash_func_num太大，增加内存使用，减少假阳性率
    // hash_func_num太小，节省内存，但是增加假阳性率
    hash_func_num = static_cast<size_t>(bits_per_key * 0.69);  // 0.69 =~ ln(2)
    if (hash_func_num < 1) hash_func_num = 1;
    if(hash_func_num > 30) hash_func_num = 30;
  }
    */

    // 直接根据 键的数量 和 要求的假阳性率 计算最佳的哈希函数数量和位数组大小
    BloomFilter::BloomFilter(int32_t keys_num, double false_positive = 0.01)
    {
        // 计算最佳的位数组大小
        int32_t bits_num = -1 * static_cast<int32_t>(std::log(false_positive) * keys_num / 0.4804530139182014);
        bits_array.resize((bits_num + 7) / 8);

        bits_num = static_cast<int>(bits_array.size()) * 8;
        bits_per_key = bits_num / keys_num;

        // 计算最佳的哈希函数数量
        hash_func_num = static_cast<int32_t>(0.6931471805599453 * bits_per_key);
        if (hash_func_num < 1)
        {
            hash_func_num = 1;
        }
        if (hash_func_num > 32)
        {
            hash_func_num = 32;
        }
    }

    std::string BloomFilter::policy_name() { return "BloomFilter"; }

    uint64_t BloomFilter::size() { return bits_array.size(); }

    void BloomFilter::create_filter(const std::vector<std::string> &keys)
    {
        uint32_t bits_size = bits_array.size() * 8;
        for (const auto &key : keys)
        {
            // murmur_hash2 哈希函数
            uint32_t h = utils::murmur_hash2(key.c_str(), key.size());
            uint32_t delta = (h >> 17) | (h << 15); // 双重哈希生成增量，用于后续的哈希计算

            // 模拟hash_func_num次哈希函数
            for (int j = 0; j < hash_func_num; j++)
            {
                uint32_t bit_pos = h % bits_size;
                bits_array[bit_pos / 8] |= (1 << (bit_pos % 8));
                h += delta;
            }
        }
    }

    bool BloomFilter::exists(const std::string_view &key)
    {
        if (key.empty())
        {
            return false;
        }
        uint32_t bits_size = bits_array.size() * 8;
        uint32_t h = utils::murmur_hash2(key.data(), key.size());
        uint32_t delta = (h >> 17) | (h << 15);
        for (int j = 0; j < hash_func_num; j++)
        {
            uint32_t bit_pos = h % bits_size;
            if ((bits_array[bit_pos / 8] & (1 << (bit_pos % 8))) == 0)
            {
                return false;
            }
            h += delta;
        }
        return true;
    }

    std::string &BloomFilter::data()
    {
        return bits_array;
    }
}