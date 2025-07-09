#pragma once

#include <format>
#include <iostream>
#include <ostream>
#include <string>
#ifndef __EMSCRIPTEN__
#include <webgpu/wgpu.h>
#endif

enum class LogLevel {
    ERROR,
    WARNING,
    INFO,
    LOG,
};


inline void log(LogLevel lvl, const std::string& message) {
    switch (lvl) {
        case LogLevel::ERROR:
            std::cerr << "ERROR: " << message << std::endl;
            break;
        case LogLevel::WARNING:
            std::cout << "WARNING: " << message << std::endl;
            break;
        case LogLevel::INFO:
            std::cout << "INFO: " << message << std::endl;
            break;
        case LogLevel::LOG:
            std::cout << message << std::endl;
            break;
    }
}

#ifndef __EMSCRIPTEN__
inline void log(WGPULogLevel lvl, const std::string& message) {
    switch (lvl) {
        case WGPULogLevel_Error:
            std::cerr << "ERROR: " << message << std::endl;
            break;
        case WGPULogLevel_Warn:
            std::cout << "WARNING: " << message << std::endl;
            break;
        case WGPULogLevel_Info:
            std::cout << "INFO: " << message << std::endl;
            break;
        default:
            std::cout << message << std::endl;
            break;
    }
}
#endif

namespace Log {
    template <typename... Args>
    inline void error(std::string_view fmt, Args&&... args) {
        auto formatted = std::vformat(fmt, std::make_format_args(args...));
        log(LogLevel::ERROR, formatted);
    }
    template <typename... Args>
    inline void warn(std::string_view fmt, Args&&... args) {
        auto formatted = std::vformat(fmt, std::make_format_args(args...));
        log(LogLevel::WARNING, formatted);
    }
    template <typename... Args>
    inline void info(std::string_view fmt, Args&&... args) {
        auto formatted = std::vformat(fmt, std::make_format_args(args...));
        log(LogLevel::INFO, formatted);
    }
    template <typename... Args>
    inline void log(std::string_view fmt, Args&&... args) {
        auto formatted = std::vformat(fmt, std::make_format_args(args...));
        log(LogLevel::LOG, formatted);
    }
}  // namespace Log
