// log.cpp
#include "log.h"

namespace lsmkv::log
{

    std::shared_ptr<spdlog::logger> logger = nullptr; // 确保使用 log
    std::mutex _mutex;

    std::shared_ptr<spdlog::logger> get_logger()
    {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        if (logger == nullptr)
        {
            logger = spdlog::stdout_color_mt("console");
            logger->set_level(spdlog::level::debug);
        }
        return logger;
    }

}
