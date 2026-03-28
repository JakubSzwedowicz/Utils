//
// Created by Jakub Szwedowicz on 10/10/25.
//

#pragma once

#include <string>
#include <unordered_map>

#include "LogLevel.h"

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
