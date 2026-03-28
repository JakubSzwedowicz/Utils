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
    void setConfig(std::shared_ptr<const Config> config) override {
        // Publish the main application config
        ConfigManager<Config, Providers...>::setConfig(config);

        if (config) {
            // Extract the LoggerConfig from the application Config
            auto newLoggerConfig = std::make_shared<Logging::LoggerConfig>(config->loggerConfig.get());

            // Check if we already have an active logger config
            auto currentLoggerConfig = ConfigPublisher<Logging::LoggerConfig>::getConfig();

            // Only publish an update if the logger configuration has actually changed
            if (!currentLoggerConfig || *currentLoggerConfig != *newLoggerConfig) {
                ConfigPublisher<Logging::LoggerConfig>::setConfig(newLoggerConfig);
            }
        }
    }
};

}  // namespace Utils::Config
