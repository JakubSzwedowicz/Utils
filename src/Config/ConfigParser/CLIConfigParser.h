//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "IConfigParser.h"
#include "Logging/Logger.h"
#include "Logging/LoggerMacros.h"

namespace Utils::Config::ConfigParser {

template <typename Config>
class CLIConfigParser : public IConfigParser<Config, std::pair<int, char**>> {
   public:
    std::shared_ptr<Config> readConfig(std::pair<int, char**> args) const override {
        auto config = std::make_shared<Config>();
        const auto parsed = parseArgs(args.first, args.second);

        for (size_t i = 0; i < config->m_container.size(); ++i) {
            auto& slot = config->m_container.at(i);
            auto it = parsed.find(std::string(slot.name()));
            if (it == parsed.end()) continue;

            if (!slot.setFromString(it->second)) {
                LOG_W("Failed to parse '{}' for parameter '{}'", it->second, slot.name());
            }
        }
        return config;
    }

   private:
    // Parses argv into { param_name -> value_string }.
    // Supported formats:
    //   --name=value
    //   --name value   (only when the next token does not start with '-')
    //   --flag         (boolean flag; stored as "true")
    // Hyphens in names are normalised to underscores to match slot names.
    static std::unordered_map<std::string, std::string> parseArgs(int argc, char** argv) {
        std::unordered_map<std::string, std::string> result;
        for (int i = 1; i < argc; ++i) {
            std::string_view arg(argv[i]);
            if (!arg.starts_with("--")) continue;
            arg.remove_prefix(2);

            std::string paramName;
            std::string paramValue;

            if (const auto eq = arg.find('='); eq != std::string_view::npos) {
                paramName = std::string(arg.substr(0, eq));
                paramValue = std::string(arg.substr(eq + 1));
            } else if (i + 1 < argc && !std::string_view(argv[i + 1]).starts_with('-')) {
                paramName = std::string(arg);
                paramValue = std::string(argv[++i]);
            } else {
                paramName = std::string(arg);
                paramValue = "true";
            }

            std::ranges::replace(paramName, '-', '_');
            result.emplace(std::move(paramName), std::move(paramValue));
        }
        return result;
    }

    mutable Utils::Logging::Logger m_logger{"CLIConfigParser"};
};

}  // namespace Utils::Config::ConfigParser
