#include "../utils/status.h"
#include <string>

#ifndef FILE_READER_H
#define FILE_READER_H

namespace lsmkv
{
    class FileReader final
    {
    public:
        explicit FileReader(const std::string &file_path);
        ~FileReader();

        DBStatus read(void *buf, int32_t count, int32_t offset) const;

    private:
        int fd{0};
    };
}

#endif