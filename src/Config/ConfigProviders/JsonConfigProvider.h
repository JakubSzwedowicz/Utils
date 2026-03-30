//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "ConfigParser/JsonConfigParser.h"
#include "ConfigProviders/IConfigProvider.h"
#include "Providers/ISourceProvider.h"
#include "Providers/ResourceProvider.h"

namespace Utils::Config::ConfigProviders {

template <typename Config>
class JsonConfigProvider : public IConfigProvider<Config> {
   public:
    explicit JsonConfigProvider(std::unique_ptr<Utils::Providers::ISourceProvider<std::string>> source)
        : m_resource(std::move(source), std::make_unique<ConfigParser::JsonConfigParser<Config>>()) {}

    void run() override {
        m_resource.run();
        if (auto cfg = m_resource.poll()) {
            m_lastConfig = std::move(*cfg);
            m_hasNew = true;
        }
    }

    std::optional<std::shared_ptr<Config>> poll() override {
        if (m_hasNew) {
            m_hasNew = false;
            return m_lastConfig;
        }
        return std::nullopt;
    }

    std::shared_ptr<const Config> getConfig() const override { return m_lastConfig; }
    std::string_view name() const override { return "JsonConfigProvider"; }

   private:
    Utils::Providers::ResourceProvider<std::shared_ptr<Config>, std::string> m_resource;
    std::shared_ptr<Config> m_lastConfig;
    bool m_hasNew{false};
};

}  // namespace Utils::Config::ConfigProviders
