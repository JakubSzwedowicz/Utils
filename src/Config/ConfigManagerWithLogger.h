//
// Created by Jakub Szwedowicz on 10/10/25.
//

#pragma once

#include "ConfigManager.h"
#include "ConfigPublisher.h"
#include "Logging/LoggerConfig.h"

namespace Utils::Config {

template <typename Config, typename... Providers>
class ConfigManagerWithLogger : public ConfigManager<Config, Providers...>,
                                public ConfigPublisher<Logging::LoggerConfig> {
   public:
    using ConfigManager<Config, Providers...>::ConfigManager;

    void setConfig(std::shared_ptr<const Config> config) override {
        ConfigManager<Config, Providers...>::setConfig(config);

        if (config) {
            auto newLoggerConfig = std::make_shared<Logging::LoggerConfig>(config->loggerConfig.get());
            auto current = ConfigPublisher<Logging::LoggerConfig>::getConfig();
            if (!current || *current != *newLoggerConfig) {
                ConfigPublisher<Logging::LoggerConfig>::setConfig(newLoggerConfig);
            }
        }
    }

   protected:
    void beforeLogging(const Config& resolved) override {
        auto newLoggerConfig = std::make_shared<Logging::LoggerConfig>(resolved.loggerConfig.get());
        auto current = ConfigPublisher<Logging::LoggerConfig>::getConfig();
        if (!current || *current != *newLoggerConfig) {
            ConfigPublisher<Logging::LoggerConfig>::setConfig(newLoggerConfig);
        }
    }
};

}  // namespace Utils::Config
