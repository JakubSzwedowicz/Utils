//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <string_view>

#include "ConfigParser/CLIConfigParser.h"
#include "Providers/IConfigProvider.h"

namespace Utils::Config::Providers {

template <typename Config>
class CLIConfigProvider : public IConfigProvider<Config> {
   public:
    void update(int argc, char** argv) {
        m_config = m_parser.readConfig({argc, argv});
    }

    std::shared_ptr<Config> getConfig() const override { return m_config; }
    std::string_view name() const override { return "CLIConfigProvider"; }

   private:
    ConfigParser::CLIConfigParser<Config> m_parser;
    std::shared_ptr<Config> m_config;
};

}  // namespace Utils::Config::Providers
