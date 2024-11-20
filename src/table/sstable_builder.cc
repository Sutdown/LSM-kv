#include <cassert>
#include "sst_config.h"
#include "utils/codec.h"
#include "footer_builder.h"
#include "sstable_builder.h"
#include "file/file_writer.h"
#include "data_block_builder.h"
#include "filter/bloom_filter.h"
#include "index_block_builder.h"
#include "filter_block_builder.h"

namespace lsmkv
{
    SSTableBuilder::SSTableBuilder(int32_t key_num, std::shared_ptr<FileWriter> _filewriter,
                                   double false_positive) : fileWriter(std::move(_filewriter))
    {
        _keys_num = key_num;
        _false_positive = false_positive;

        dataBlockBuilder = std::make_shared<DataBlockBuilder>();
        filterBlockBuilder = std::make_shared<FilterBlockBuilder>(key_num, false_positive);
        indexBlockBuilder = std::make_shared<IndexBlockBuilder>();
        footerBuilder = std::make_shared<FooterBuilder>();
        logger = log::get_logger();
    }

    DBStatus SSTableBuilder::add(const std::string &key, const std::string val)
    {
        if (key.empty())
        {
            return Status::Success;
        }

        filterBlockBuilder->add(key);
        dataBlockBuilder->add(key, val);
        key_count++;
        pre_key = key;

        // datablockbuilder大小超过限制,持久化原先的数据块，然后清空
        if (dataBlockBuilder->size() > SSTConfigInfo::MAX_DATA_BLOCK_SIZE)
        {
            // 刷新到磁盘，写入文件
            dataBlockBuilder->finish_data_block();
            fileWriter->append(dataBlockBuilder->data().data(),
                               static_cast<int32_t>(dataBlockBuilder->data().size()));
            // 更新filter,filter的开始在block的末尾
            FilterBlock_offset.offset += static_cast<int32_t>(dataBlockBuilder->data().size());

            // 给持久化的datablock生成index信息
            // 持久化相应的index，然后清空
            auto shortest_key = find_shortest_key(pre_key, key);
            OffsetInfo data_block_offset_info{};
            data_block_offset_info.size = static_cast<int32_t>(dataBlockBuilder->data().size());
            data_block_offset_info.offset = FilterBlock_offset.offset;
            indexBlockBuilder->add(shortest_key, data_block_offset_info);
            index_data.append(indexBlockBuilder->data());
            indexBlockBuilder->clear();

            dataBlockBuilder->clear();
        }
        return Status::Success;
    }

    DBStatus SSTableBuilder::finish_sst()
    {
        // 最后一个datablock时
        if (dataBlockBuilder->size() > 0)
        {
            // 写到文件
            dataBlockBuilder->finish_data_block();
            fileWriter->append(dataBlockBuilder->data().data(), static_cast<int32_t>(dataBlockBuilder->data().size()));

            // 保存index block
            OffsetInfo offset_info{};
            offset_info.size = static_cast<int32_t>(dataBlockBuilder->data().size());
            offset_info.offset = FilterBlock_offset.offset;

            // 保存index data
            pre_key.back() += 1;
            indexBlockBuilder->add(pre_key, offset_info);
            index_data.append(indexBlockBuilder->data());
            indexBlockBuilder->clear();

            FilterBlock_offset.offset += static_cast<int32_t>(dataBlockBuilder->data().size());
            dataBlockBuilder->clear();
        }

        filterBlockBuilder->finish_filter_block();
        // 保存filiter大小
        FilterBlock_offset.size = static_cast<int32_t>(filterBlockBuilder->data().size());

        // 保存filiter
        filterBlockBuilder->finish_filter_block();
        fileWriter->append(filterBlockBuilder->data().data(), static_cast<int32_t>(filterBlockBuilder->data().size()));

        // 持久化index data
        fileWriter->append(index_data.c_str(), static_cast<int32_t>(index_data.size()));

        // 持久化footer
        assert(static_cast<int32_t>(index_data.size()) > 0);
        IndexBlock_offset.offset = FilterBlock_offset.offset + FilterBlock_offset.size;
        IndexBlock_offset.size = static_cast<int32_t>(index_data.size());
        footerBuilder->add(FilterBlock_offset, IndexBlock_offset);
        fileWriter->append(footerBuilder->data().data(), static_cast<int32_t>(footerBuilder->data().size()));

        fileWriter->flush();
        fileWriter->close();

        // 清空,filewriter的指针会自动销毁
        dataBlockBuilder->clear();
        indexBlockBuilder->clear();
        filterBlockBuilder = std::make_shared<FilterBlockBuilder>(_keys_num, _false_positive);
        footerBuilder->clear();

        FilterBlock_offset.clear();
        IndexBlock_offset.clear();
        index_data.clear();
        pre_key.clear();
        pre_offset_info.clear();

        return Status::Success;
    }

    // 生成一个比key1稍大点的键
    std::string SSTableBuilder::find_shortest_key(const std::string &key1, const std::string &key2)
    {
        assert(!key1.empty());
        assert(!key2.empty());
        assert(key1 < key2);

        int cnt = 0;
        for (int i = 0; i < std::min(key1.size(), key2.size()); i++)
        {
            if (key1[i] == key2[i])
            {
                ++cnt;
            }
        }

        std::string shortest_key;
        if (key1.size() == cnt)
        {
            shortest_key = key1 + ' ';
            return shortest_key;
        }
        shortest_key = key1.substr(0, cnt + 1);
        shortest_key.back() += 1;
        return shortest_key;
    }
}