//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <optional>
#include <string_view>

#include "ConfigParser/CLIConfigParser.h"
#include "ConfigProviders/IConfigProvider.h"

namespace Utils::Config::ConfigProviders {

template <typename Config>
class CLIConfigProvider : public IConfigProvider<Config> {
   public:
    CLIConfigProvider(int argc, char** argv) : m_config(m_parser.parse({argc, argv})) {}

    void run() override {}
    std::optional<std::shared_ptr<Config>> poll() override { return std::nullopt; }

    std::shared_ptr<const Config> getConfig() const override { return m_config; }
    std::string_view name() const override { return "CLIConfigProvider"; }

   private:
    ConfigParser::CLIConfigParser<Config> m_parser;
    std::shared_ptr<Config> m_config;
};

}  // namespace Utils::Config::ConfigProviders
