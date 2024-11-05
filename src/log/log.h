// log.h
#ifndef LOG_H
#define LOG_H

#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
#include <mutex>

namespace lsmkv::logger
{
    class log
    {
    public:
        static std::shared_ptr<spdlog::logger> get_instance();

    private:
        log() = default;
        static std::shared_ptr<spdlog::logger> _instance;
        static std::mutex _mutex;
    };
}

#endif // LOG_H
