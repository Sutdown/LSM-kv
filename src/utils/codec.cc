#include "codec.h"

namespace lsmkv
{
    namespace utils
    {
        void EncodeFixed32(char *buf, uint32_t val)
        {
            buf[0] = val & 0xff;
            buf[1] = (val >> 8) & 0xff;
            buf[2] = (val >> 16) & 0xff;
            buf[3] = (val >> 24) & 0xff;
        }

        uint32_t DecodeFixed32(const char *data)
        {
            auto _data = reinterpret_cast<const uint8_t *>(data);
            return static_cast<uint32_t>(_data[0]) |
                   (static_cast<uint32_t>(_data[1]) << 8) |
                   (static_cast<uint32_t>(_data[2]) << 16) |
                   (static_cast<uint32_t>(_data[3]) << 24);
        }

        uint64_t DecodeFixed64(const char *data)
        {
            auto _data = reinterpret_cast<const uint8_t *>(data);
            return static_cast<uint64_t>(_data[0]) |
                   (static_cast<uint64_t>(_data[1]) << 8) |
                   (static_cast<uint64_t>(_data[2]) << 16) |
                   (static_cast<uint64_t>(_data[3]) << 24) |
                   (static_cast<uint64_t>(_data[4]) << 32) |
                   (static_cast<uint64_t>(_data[5]) << 40) |
                   (static_cast<uint64_t>(_data[6]) << 48) |
                   (static_cast<uint64_t>(_data[7]) << 56);
        }
    }
}