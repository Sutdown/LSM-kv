#include <cstdint>
#include "db/status.h"
#include <string>

#ifndef FILE_WRITER_H
#define FILE_WRITER_H

namespace lsmkv
{
    class FileWriter final
    {
    private:
        static constexpr int32_t BUFFER_SIZE = 65536;
        int32_t buffer_offset = 0;
        char buffer[BUFFER_SIZE]{};

        int32_t fd;

    public:
        FileWriter(const std::string &file_path, bool append = true);
        ~FileWriter();

        // 默认直接追加到buffer，当开启flush=true，追加后立即刷新磁盘
        DBStatus append(const char *data, int32_t len, bool flush = false);

        // 将buf写道内核缓冲区
        DBStatus flush();

        // 内核缓冲区到磁盘
        void sync();

        void close();

    private:
        // 实现持久化
        DBStatus buf_persist(const char *data, int32_t len);

        bool is_buffer_full() const
        {
            return buffer_offset == BUFFER_SIZE;
        }
    };
}

#endif