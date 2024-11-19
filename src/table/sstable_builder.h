#include <memory>
#include <string>
#include <string_view>
#include "../log/log.h"
#include "../db/status.h"
#include "../db/offset_info.h"

#ifndef SSTABLE_BUILDER_H
#define SSTABLE_BUILDER_H

namespace lsmkv
{
    class DataBlockBuilder;
    class FilterBlockBuilder;
    class IndexBlockBuilder;
    class FooterBuilder;

    class FileWriter;

    class SSTableBuilder final
    {
    private:
        std::shared_ptr<DataBlockBuilder> dataBlockBuilder = nullptr;
        std::shared_ptr<FilterBlockBuilder> filterBlockBuilder = nullptr;
        std::shared_ptr<IndexBlockBuilder> indexBlockBuilder = nullptr;
        std::shared_ptr<FooterBuilder> footerBuilder = nullptr;

        std::shared_ptr<FileWriter> fileWriter = nullptr;

        std::shared_ptr<spdlog::logger> logger;

        // filter
        int32_t _keys_num = 0;
        double _false_positive = 0;

        // index block
        std::string index_data;

        // data block
        int32_t key_count = 0;
        std::string pre_key;
        OffsetInfo pre_offset_info{};

        // 偏移量，将footer和index和block，filter联系起来
        OffsetInfo FilterBlock_offset{0, 0};
        OffsetInfo IndexBlock_offset{0, 0};

    public:
        // filter数据，filewrite
        explicit SSTableBuilder(int32_t key_num, std::shared_ptr<FileWriter> _filewriter,
                                double false_positive = 0.01);
        ~SSTableBuilder() = default;

        DBStatus add(const std::string &key, const std::string val);
        DBStatus finish_sst(); // 完整写入磁盘

    private:
        static std::string find_shortest_key(const std::string &key1, const std::string &key2);
    };
}

#endif