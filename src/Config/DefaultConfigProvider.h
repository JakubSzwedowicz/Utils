//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <string_view>

#include "IConfigProvider.h"

namespace Utils::Config {

template <typename Config>
class DefaultConfigProvider : public IConfigProvider<Config> {
   public:
    DefaultConfigProvider() : m_config(std::make_shared<Config>()) { m_config->m_container.applyDefaults(); }

    std::shared_ptr<Config> getConfig() const override { return m_config; }
    std::string_view name() const override { return "default"; }

   private:
    std::shared_ptr<Config> m_config;
};

}  // namespace Utils::Config
