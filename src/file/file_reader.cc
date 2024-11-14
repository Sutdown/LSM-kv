#include <string>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>
#include "../log/log.h"
#include "file_reader.h"

namespace lsmkv
{
    FileReader::FileReader(const std::string &file_path)
    {
        if (access(file_path.c_str(), F_OK) != F_OK)
        {
            log::get_instance()->error("{} cannot access.", file_path);
        }
        else
        {
            fd = open(file_path.c_str(), O_RDONLY);
            if (fd < 0)
            {
                log::get_instance()->error("{} cannot open.", file_path);
            }
        }
    }

    FileReader::~FileReader()
    {
        if (fd != -1)
        {
            close(fd);
            fd = -1;
        }
    }

    DBStatus FileReader::read(void *buf, int32_t count, int32_t offset) const
    {
        // 缓冲区为空
        if (buf == nullptr)
        {
            return Status::InvalidArgs;
        }
        // 读取失败
        if (fd == -1)
        {
            log::get_instance()->error("fd == -1");
            return Status::ExecFailed;
        }
        // 可以在多线程环境下安全的实现并发读取
        auto cnt = pread(fd, buf, count, offset);
        if (cnt != count)
        {
            log::get_instance()->error("pread exec failed.");
            return Status::ExecFailed;
        }
        return Status::Success;
    }
}