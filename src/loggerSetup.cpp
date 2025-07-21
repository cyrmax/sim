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
constexpr const char* LOG_FORMAT = "%R Level: %l, Thread: %t, Message: %v";

void InitializeLogging(int argc, char* argv[], bool isDebuggingEnabled) {
    try {
#ifndef NDEBUG
        bool isDebugBuild = true;
#else
        bool isDebugBuild = false;
#endif
        // spdlog::cfg::load_env_levels();
        // spdlog::cfg::load_argv_levels(argc, argv);
        spdlog::init_thread_pool(LOGGER_THREAD_POOL_QUEUE_SIZE, LOGGER_THREAD_POOL_BACKING_THREAD_COUNT);
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("sim.log", true);
        std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
        auto logger = std::make_shared<spdlog::async_logger>(
            "simlogger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        logger->set_pattern(LOG_FORMAT);
        if (isDebuggingEnabled || isDebugBuild) {
            logger->set_level(spdlog::level::trace);
        } else {
            logger->set_level(spdlog::level::warn);
        }
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        if (isDebugBuild) {
            spdlog::debug("Log level is set to trace because of debug build");
        }
        if (isDebuggingEnabled) {
            spdlog::debug("Log level is set to debug via command line parameter");
        }
        std::atexit(spdlog::shutdown);
    } catch (spdlog::spdlog_ex& ex) { std::cerr << "Unable to initialize logger: " << ex.what() << '\n'; }
}
