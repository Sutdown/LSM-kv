// main.cpp
#include "log/log.h"

#include <stack>
#include <queue>
#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <memory>
#include <cassert>
#include <cmath>
#include "utils/murmur_hash2.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "log/log.h"
#include "filter/filter_policy.h"
#include "filter/bloom_filter.h"

using namespace std;

int main()
{
    auto logger = lsmkv::logger::log::get_instance();
    std::unique_ptr<lsmkv::FilterPolicy> filterPolicy = std::make_unique<lsmkv::BloomFilter>(10 * 10000, 0.01);
    std::vector<std::string> data;
    // 插入10w
    for (int i = 0; i < 10 * 10000; ++i)
    {
        data.push_back("key_" + std::to_string(i));
    }
    filterPolicy->create_filter(data);
    int cnt = 0;
    for (int i = 0; i < 20 * 10000; ++i)
    {
        if (filterPolicy->exists("key_" + std::to_string(i)))
        {
            ++cnt;
        }
    }
    logger->info("cnt=" + to_string(cnt));

    return 0;
}