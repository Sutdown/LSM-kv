// log.cpp
#include "log.h"

std::shared_ptr<spdlog::logger> log::_instance = nullptr; // 确保使用 log
std::mutex log::_mutex;

std::shared_ptr<spdlog::logger> log::get_instance()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_instance)
    {
        _instance = spdlog::stdout_color_mt("console");
        _instance->set_level(spdlog::level::debug);
    }
    return _instance;
}
