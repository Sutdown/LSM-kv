#include <cassert>
#include "sst_config.h"
#include "data_block_builder.h"
#include "../utils/codec.h"
#include "../log/log.h"

namespace lsmkv
{
    DBStatus DataBlockBuilder::add(const std::string &key, const std::string &value)
    {
        if (key.empty())
        {
            return Status::Success;
        }

        bool need_fullykey = false;
        // 观察当前记录数量是否到达最大需要重启的数量
        if (record_num % SSTConfigInfo::RESTART_INTERVAL == 0)
        {
            need_fullykey = true;
            record_group_offset.push_back(static_cast<int32_t>(get_records_size()));
        }
        ++record_num;

        // 需要重启，不需要差值压缩
        if (need_fullykey)
        {
            utils::PutFixed32(_data, 0);                                  // shared_key_len
            utils::PutFixed32(_data, static_cast<int32_t>(key.size()));   // unshared_key_len
            utils::PutFixed32(_data, static_cast<int32_t>(value.size())); // value_len
            _data.append(key);                                            // unshared_key_content
            _data.append(value);                                          // value_content
        }
        else
        {
            // 需要差值压缩
            auto min_len = std::min(key.size(), pre_key.size());
            int shared_key_len = 0;
            for (int i = 0; i < min_len; ++i)
            {
                if (key[i] == pre_key[i])
                {
                    ++shared_key_len;
                }
                else
                {
                    break;
                }
            }

            // 可以采用variant编码
            int unshared_key_len = static_cast<int>(key.size()) - shared_key_len;
            utils::PutFixed32(_data, shared_key_len);
            utils::PutFixed32(_data, unshared_key_len);
            utils::PutFixed32(_data, static_cast<int32_t>(value.size()));
            _data.append(key.substr(shared_key_len));
            _data.append(value);
        }

        pre_key = key;
        return Status::Success;
    }

    // 将最终数据块中的重启点信息写入data
    DBStatus DataBlockBuilder::finish_data_block()
    {
        int restart_point_num = static_cast<int>((record_num - 0.5) / 16) + 1;
        int last_offset = static_cast<int>(_data.size()); // 记录_data的最后一个字节的位置

        for (int i = 0; i < record_group_offset.size(); ++i)
        {
            int32_t _record_num; // 当前记录组中的记录数量
            OffsetInfo _offsetInfo{0, record_group_offset[i]};
            if (i != record_group_offset.size() - 1)
            {
                _offsetInfo.size = record_group_offset[i + 1] - record_group_offset[i];
                _record_num = SSTConfigInfo::RESTART_INTERVAL;
            }
            else
            {
                _offsetInfo.size = last_offset - record_group_offset[i];
                _record_num = record_num % SSTConfigInfo::RESTART_INTERVAL;
            }

            utils::PutFixed32(_data, _record_num);
            utils::PutFixed64(_data, *reinterpret_cast<int64_t *>(&_offsetInfo));

            // 重启点数量
            utils::PutFixed32(_data, restart_point_num);
            return Status::Success;
        }
    }

    void DataBlockBuilder::clear()
    {
        record_group_offset.clear();
        pre_key.clear();
        _data.clear();
        record_num = 0;
    }
}