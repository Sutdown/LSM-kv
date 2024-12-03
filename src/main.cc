#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include <string_view>
#include "./db/db.h" // 引入 DB 接口
#include "./db/status.h"

using namespace lsmkv;

// 用于生成随机字符串的辅助函数
std::string GenerateRandomString(size_t length)
{
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string random_str;
    for (size_t i = 0; i < length; ++i)
    {
        random_str += charset[dist(rng)];
    }
    return random_str;
}

// 压测函数：插入和读取 10 万条数据
void BenchmarkDBInsertionAndRead(int num_entries = 100000)
{
    // 配置选项
    lsmkv::Options options;
    // 根据实际情况配置 options，这里假设有一些默认设置
    lsmkv::DB db(options);

    // 写入选项
    lsmkv::WriteOptions write_options;

    // 读取选项
    lsmkv::ReadOptions read_options;

    // 插入数据并测量时间
    auto start_insert = std::chrono::high_resolution_clock::now();

    // 插入数据
    for (int i = 0; i < num_entries; ++i)
    {
        std::string key = "key_" + std::to_string(i);  // 生成key
        std::string value = GenerateRandomString(100); // 生成100字节的随机value
        lsmkv::DBStatus status = db.Put(write_options, key, value);
        if (status != Status::Success)
        {
            std::cerr << "Insert failed at index " << i << std::endl;
            return;
        }
    }

    auto end_insert = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> insert_duration = end_insert - start_insert;
    std::cout << "Time to insert " << num_entries << " entries: " << insert_duration.count() << " seconds" << std::endl;

    // 读取数据并测量时间
    auto start_read = std::chrono::high_resolution_clock::now();

    // 读取数据
    for (int i = 0; i < num_entries; ++i)
    {
        std::string key = "key_" + std::to_string(i);
        std::string ret_value;
        lsmkv::DBStatus status = db.Get(read_options, key, &ret_value);
        if (status != Status::Success)
        {
            std::cerr << "Read failed at index " << i << std::endl;
            return;
        }
    }

    auto end_read = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> read_duration = end_read - start_read;
    std::cout << "Time to read " << num_entries << " entries: " << read_duration.count() << " seconds" << std::endl;
}

int main()
{
    BenchmarkDBInsertionAndRead(100000); // 插入和读取 10 万条数据
    return 0;
}
