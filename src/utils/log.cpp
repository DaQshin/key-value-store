#include <utils/log.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace logs
{
    void init(const std::string &log_path, spdlog::level::level_enum level)
    {
        spdlog::init_thread_pool(65536, 1);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_path, 100 * 1024 * 1024, 5);

        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        auto logger = std::make_shared<spdlog::async_logger>(
            "kvstore",
            sinks.begin(), sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::overrun_oldest);

        spdlog::set_default_logger(logger);
        spdlog::set_level(level);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [tid %t] %v");
        spdlog::flush_on(spdlog::level::warn);
    }
} // namespace log