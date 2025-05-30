#include "loggerSetup.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <spdlog/async.h>
#include <spdlog/cfg/argv.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

constexpr int LOGGER_THREAD_POOL_QUEUE_SIZE = 8192;
constexpr int LOGGER_THREAD_POOL_BACKING_THREAD_COUNT = 1;
#ifndef NDEBUG
constexpr const char* LOG_FORMAT = "%R Level: %l, Thread: %t, Message: %v";
#else
constexpr const char* LOG_FORMAT = "%R Level: %l, Thread: %t, Message: %v";
#endif

void InitializeLogging(int argc, char* argv[]) {
    try {
#ifdef DEBUG
        spdlog::set_level(spdlog::level::debug);
#else
        spdlog::set_level(spdlog::level::warn);
#endif
        spdlog::cfg::load_env_levels();
        spdlog::cfg::load_argv_levels(argc, argv);
        spdlog::init_thread_pool(LOGGER_THREAD_POOL_QUEUE_SIZE, LOGGER_THREAD_POOL_BACKING_THREAD_COUNT);
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("sim.log", true);
        std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
        auto logger = std::make_shared<spdlog::async_logger>(
            "simlogger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        logger->set_level(spdlog::level::trace);
        logger->set_pattern(LOG_FORMAT);
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    } catch (spdlog::spdlog_ex& ex) { std::cerr << "Unable to initialize logger: " << ex.what() << '\n'; }
}
