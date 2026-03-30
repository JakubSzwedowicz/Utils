//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <optional>
#include <string_view>

#include "ConfigProviders/IConfigProvider.h"

namespace Utils::Config::ConfigProviders {

template <typename Config>
class DefaultConfigProvider : public IConfigProvider<Config> {
   public:
    DefaultConfigProvider() : m_config(std::make_shared<Config>()) { m_config->m_container.applyDefaults(); }

    void run() override {}
    std::optional<std::shared_ptr<Config>> poll() override { return std::nullopt; }
    std::shared_ptr<const Config> getConfig() const override { return m_config; }
    std::string_view name() const override { return "default"; }

   private:
    std::shared_ptr<Config> m_config;
};

}  // namespace Utils::Config::ConfigProviders
