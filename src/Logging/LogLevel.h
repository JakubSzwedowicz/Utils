//
// Created by Jakub Szwedowicz on 10/10/25.
//

#pragma once
#include <cstdint>
#include <string>

namespace Utils::Logging {
enum class LogLevel : uint8_t { DEBUG, INFO, WARNING, ERROR, CRITICAL, OFF, _COUNT };

static inline std::string toString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::CRITICAL:
            return "CRITICAL";
        case LogLevel::OFF:
            return "OFF";
        case LogLevel::_COUNT:
            return "COUNT";
    }
}

}  // namespace Utils::Logging
