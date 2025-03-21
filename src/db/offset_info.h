#include <cstdint>

#ifndef OFFSET_INFO_H
#define OFFSET_INFO_H

namespace lsmkv
{
    struct OffsetInfo
    {
        /* sstable中的偏移量 */
        int32_t size = 0;
        int32_t offset = 0;

        int32_t get_offset_info_size()
        {
            return sizeof(size) + sizeof(offset);
        }

        void clear()
        {
            size = 0;
            offset = 0;
        }
    };
}

#endif