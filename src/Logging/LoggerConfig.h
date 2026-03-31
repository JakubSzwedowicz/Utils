//
// Created by Jakub Szwedowicz on 10/10/25.
//

#pragma once

#include <ostream>
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
        return filename == other.filename && globalLogLevel == other.globalLogLevel &&
               loggersLogLevels == other.loggersLogLevels;
    }

    bool operator!=(const LoggerConfig& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const LoggerConfig& lc);
};

static inline std::string toString(const LoggerConfig& lc) {
    std::string res = "{filename: " + lc.filename + ", globalLogLevel: " + Utils::Logging::toString(lc.globalLogLevel);
    if (!lc.loggersLogLevels.empty()) {
        res += ", loggersLogLevels: {";
        bool first = true;
        for (const auto& [name, lvl] : lc.loggersLogLevels) {
            if (!first) res += ", ";
            res += name + ": " + Utils::Logging::toString(lvl);
            first = false;
        }
        res += "}";
    }
    return res + "}";
}

inline std::ostream& operator<<(std::ostream& os, const LoggerConfig& lc) {
    os << toString(lc);
    return os;
}

}  // namespace Utils::Logging
