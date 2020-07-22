
#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES                                                                                                                 \
    {                                                                                                                                      \
        "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "HALT", "OFF"                                                                    \
    }
#endif

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <alt_rotating_sink.h>

namespace logger 
{
    inline void init()
    {
        auto app_logger = alt_rotating_logger_mt<spdlog::async_factory>("app_logger", "example.log", 1024);
        spdlog::set_default_logger(app_logger);
        spdlog::set_pattern("%Y/%m/%d %T.%e | %^%l%$ | %t | %@ | %! | %v");
        spdlog::flush_on(spdlog::level::debug);
        spdlog::set_level(spdlog::level::debug);
    }
}

#include <unistd.h>

int main()
{
    logger::init();

    uint64_t max_row = 1024;

    for (uint64_t row = 1; row <= max_row; ++row)
    {
        SPDLOG_INFO("ROW {} / {}", row, max_row);

        if (0 == (row % 7))
            usleep(1000);
    }
}
