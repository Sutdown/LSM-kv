#include "index_block_builder.h"
#include "../utils/codec.h"
#include "../file/file_writer.h"

namespace lsmkv
{
    DBStatus IndexBlockBuilder::add(const std::string &shortest_key, const OffsetInfo &offsetInfo)
    {
        utils::PutFixed32(_data, shortest_key.size());
        _data.append(shortest_key);

        OffsetInfo _offsetInfo = offsetInfo;
        // 强制类型转换，可能存在一定风险
        utils::PutFixed64(_data, *reinterpret_cast<uint64_t *>(&_offsetInfo));
        return Status::Success;
    }
}