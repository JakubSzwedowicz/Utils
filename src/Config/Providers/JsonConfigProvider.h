//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <istream>
#include <memory>
#include <string_view>

#include "ConfigParser/JsonConfigParser.h"
#include "Providers/IConfigProvider.h"

namespace Utils::Config::Providers {

template <typename Config>
class JsonConfigProvider : public IConfigProvider<Config> {
   public:
    void update(std::istream& stream) {
        m_config = m_parser.readConfig(stream);
    }

    std::shared_ptr<Config> getConfig() const override { return m_config; }
    std::string_view name() const override { return "JsonConfigProvider"; }

   private:
    ConfigParser::JsonConfigParser<Config> m_parser;
    std::shared_ptr<Config> m_config;
};

}  // namespace Utils::Config::Providers
