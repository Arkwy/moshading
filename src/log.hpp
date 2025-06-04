#pragma once

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
inline void error(const std::string& message) { log(LogLevel::ERROR, message); }
inline void warn(const std::string& message) { log(LogLevel::WARNING, message); }
inline void info(const std::string& message) { log(LogLevel::INFO, message); }
inline void log(const std::string& message) { log(LogLevel::LOG, message); }
}  // namespace Log
