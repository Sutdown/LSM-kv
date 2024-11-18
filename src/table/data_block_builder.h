#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <string_view>
#include "../db/status.h"
#include "../db/offset_info.h"

#ifndef DATA_BLOCK_BUILDER_H
#define DATA_BLOCK_BUILDER_H
namespace lsmkv
{
    class FileWriter;
    class DataBlockBuilder final
    {
        using size_type = std::string::size_type;

    private:
        int32_t record_num = 0;                   // 记录点
        std::vector<int32_t> record_group_offset; // 保存每个记录的偏移量
        std::string pre_key;
        std::string _data; // 当前数据

    public:
        DataBlockBuilder() = default;
        ~DataBlockBuilder() = default;

        DBStatus add(const std::string &key, const std::string &value);

        // 当block中的record大小超过阈值时，停止写入，生成重启点等信息
        // 该函数调用完成说明当前datablock已经完全写完
        // 所有数据都写到了块中，只需要持久化_data即可
        DBStatus finish_data_block();

        std::string_view data() { return _data; }

        void clear();

        size_type size() { return _data.size(); }

    private:
        size_type get_records_size() { return _data.size(); }
    };
}

#endif