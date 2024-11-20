#include <memory>
#include <string_view>
#include <utility>
#include "../db/status.h"
#include "log_format.h"

#ifndef WAL_WRITER_H
#define WAL_WRITER_H

namespace lsmkv
{
    class FileWriter;
    class WALWriter final
    {
    public:
        // 接收FileWriter作为参数，是为了遵循单一职责原则(SRP)和依赖注入原则(DI)
        explicit WALWriter(std::shared_ptr<FileWriter> writableFile) : writableFile(std::move(writableFile)) {};
        ~WALWriter();

        DBStatus ADDLog(const std::string_view &log, bool need_flush = true);

    private:
        std::shared_ptr<FileWriter> writableFile;
    };
}

#endif