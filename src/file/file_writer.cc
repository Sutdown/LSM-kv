#include <filesystem>
#include <cassert>
#include <unistd.h>
#include "file_writer.h"
#include "log/log.h"
#include "config/global_var.h"

namespace lsmkv
{
    FileWriter::FileWriter(const std::string &file_path, bool append)
    {
        int mode = O_CREAT | O_WRONLY; // 文件以只写模式打开
        if (append)
        { // 文件内容追加
            mode |= O_APPEND;
        }
        else
        { // 清空文件
            mode |= O_TRUNC;
        }

        // 文件目录不存在时
        if (access(file_path.c_str(), F_OK) != F_OK)
        {
            int idx = (int)file_path.rfind('/');
            std::string dir_path = file_path.substr(0, idx);
            std::filesystem::create_directories(dir_path.c_str());
        }
        this->fd = open(file_path.c_str(), mode, 0644);
        assert(access(file_path.c_str(), F_OK) == F_OK);
    }

    FileWriter::~FileWriter()
    {
        sync();
    }

    // 默认直接追加到buffer，当开启flush=true，追加后立即刷新磁盘
    DBStatus FileWriter::append(const char *data, int32_t len, bool flush)
    {
        int32_t data_offset = 0;
        assert(data != nullptr);
        if (len == 0)
        {
            return Status::Success;
        };

        int remain_buf_size = BUFFER_SIZE - buffer_offset;
        if (len < remain_buf_size)
        {
            // 缓冲区足够
            memcpy(buffer + buffer_offset, data + data_offset, len);
            buffer_offset += len;
            data_offset += len;
        }
        else
        {
            // 缓冲区不够
            memcpy(buffer + buffer_offset, data, remain_buf_size);
            len -= remain_buf_size;
            buffer_offset += remain_buf_size;
            data_offset += remain_buf_size;
            auto rsp = buf_persist(buffer, BUFFER_SIZE);
            assert(rsp == Status::Success);

            // 读取剩余数据
            while (len > 0)
            {
                // 观测缓冲区
                if (is_buffer_full())
                {
                    rsp = buf_persist(buffer, BUFFER_SIZE);
                    assert(rsp == Status::Success);
                }
                int need_cpy = std::min(len, BUFFER_SIZE - buffer_offset);
                memcpy(buffer + buffer_offset, data + data_offset, need_cpy);
                data_offset += need_cpy;
                len -= need_cpy;
                buffer_offset += need_cpy;
                if (len == 0)
                {
                    break;
                }
            }
        }
        if (flush)
        {
            buf_persist(buffer, buffer_offset);
        }
        return Status::Success;
    }

    // 将buf写到c库缓冲
    DBStatus FileWriter::flush()
    {
        if (buffer_offset > 0)
        {
            auto rsp = buf_persist(buffer, buffer_offset);
            assert(rsp == Status::Success);
        }
        return Status::Success;
    }

    // buf到磁盘
    void FileWriter::sync()
    {
        flush();
        fsync(fd); // 将fd所指向的文件的缓冲数据同步到磁盘中
    }

    void FileWriter::close()
    {
        flush();
        ::close(fd);
        fd = -1;
    }

    DBStatus FileWriter::buf_persist(const char *data, int32_t len)
    {
        // 将数据从用户空间写到文件描述符指向的文件中
        auto ret = write(fd, data, len);
        assert(ret == len);
        buffer_offset = 0; // 用户空间的缓冲区清空
        return Status::Success;
    }
}