//
// Created by Jakub Szwedowicz on 10/10/25.
//

#pragma once

#include <string>
#include <unordered_map>

#include "LogLevel.h"
#include "glaze/glaze.hpp"

namespace Utils::Logging {

struct LoggerConfig {
    std::string filename = "mainLog.txt";
    LogLevel globalLogLevel = LogLevel::INFO;
    std::unordered_map<std::string, LogLevel> loggersLogLevels;

    bool operator==(const LoggerConfig& other) const {
        return filename == other.filename &&
               globalLogLevel == other.globalLogLevel &&
               loggersLogLevels == other.loggersLogLevels;
    }

    bool operator!=(const LoggerConfig& other) const {
        return !(*this == other);
    }
};

}  // namespace Utils::Logging

namespace glz {
template <>
struct meta<Utils::Logging::LoggerConfig> {
    using T = Utils::Logging::LoggerConfig;
    static constexpr auto value = object(
        "filename",         &T::filename,
        "globalLogLevel",   &T::globalLogLevel,
        "loggersLogLevels", &T::loggersLogLevels
    );
};
}
