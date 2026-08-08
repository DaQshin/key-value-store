#pragma once

#include <spdlog/spdlog.h>

namespace logs
{
    void init(const std::string &log_path = "logs/kvstore.log",
              spdlog::level::level_enum level = spdlog::level::info);
} // namespace log