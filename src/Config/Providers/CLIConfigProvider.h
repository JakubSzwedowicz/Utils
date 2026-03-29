//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <optional>
#include <string_view>

#include "ConfigParser/CLIConfigParser.h"
#include "Providers/IConfigProvider.h"

namespace Utils::Config::Providers {

template <typename Config>
class CLIConfigProvider : public IConfigProvider<Config> {
   public:
    void update(int argc, char** argv) {
        m_config = m_parser.parse({argc, argv});
        m_hasNew = true;
    }

    void run() override {}

    std::optional<std::shared_ptr<Config>> poll() override {
        if (m_hasNew) {
            m_hasNew = false;
            return m_config;
        }
        return std::nullopt;
    }

    std::shared_ptr<const Config> getConfig() const override { return m_config; }
    std::string_view name() const override { return "CLIConfigProvider"; }

   private:
    ConfigParser::CLIConfigParser<Config> m_parser;
    std::shared_ptr<Config> m_config;
    bool m_hasNew{false};
};

}  // namespace Utils::Config::Providers
