// main.cpp
#include "log/log.h"

int main()
{
    auto logger = log::get_instance(); // 确保类名和调用匹配
    logger->info("Hello, world!");     // 记录日志
    return 0;
}
