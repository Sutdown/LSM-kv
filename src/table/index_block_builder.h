#include <memory>
#include <string>
#include <utility>
#include "../db/offset_info.h"
#include "../db/status.h"

#ifndef INDEX_BLOCK_BUILDER_H
#define INDEX_BLOCK_BUILDER_H

namespace lsmkv
{
    class FileWriter;

    class IndexBlockBuilder final
    {
    private:
        std::string _data; // 序列化shortest key,offsetinfo
    public:
        IndexBlockBuilder() = default;
        ~IndexBlockBuilder() = default;

        DBStatus add(const std::string &shortest_key, const OffsetInfo &offsetInfo);

        inline std::string data() { return _data; }
        inline void clear() { _data.clear(); }
    };
}

#endif