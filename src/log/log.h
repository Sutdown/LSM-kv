// log.h
#ifndef LOG_H
#define LOG_H

#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
#include <mutex>

namespace lsmkv::log
{
    std::shared_ptr<spdlog::logger> get_logger();
}

#endif // LOG_H
