//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <array>
#include <memory>
#include <tuple>

#include "ConfigPublisher.h"
#include "DefaultConfigProvider.h"
#include "Logging/Logger.h"
#include "Logging/LoggerMacros.h"

namespace Utils::Config {

template <typename Config, typename... Providers>
class ConfigManager : public ConfigPublisher<Config> {
    using DefaultProvider = DefaultConfigProvider<Config>;
    static constexpr size_t kTotalProviders = sizeof...(Providers) + 1;

   public:
    ConfigManager()
        : m_providers(std::make_unique<Providers>()..., std::make_unique<DefaultProvider>()) {
        resolve();
    }

    template <typename T>
    T& getProvider() {
        return *std::get<std::unique_ptr<T>>(m_providers);
    }

    template <typename T, typename... Args>
    void update(Args&&... args) {
        getProvider<T>().update(std::forward<Args>(args)...);
        resolve();
    }

    void resolve() {
        auto resolved = std::make_shared<Config>();

        auto providerConfigs = std::apply(
            [](auto&... p) {
                return std::array<std::shared_ptr<Config>, sizeof...(p)>{p->getConfig()...};
            },
            m_providers);

        auto providerNames = std::apply(
            [](auto&... p) { return std::array<std::string_view, sizeof...(p)>{p->name()...}; },
            m_providers);

        size_t maxParamLen = 0;
        for (size_t i = 0; i < resolved->m_container.size(); ++i)
            maxParamLen = std::max(maxParamLen, resolved->m_container.at(i).name().size());

        size_t maxSourceLen = 0;
        for (auto sv : providerNames)
            maxSourceLen = std::max(maxSourceLen, sv.size());

        LOG_I("Resolving config [{} provider(s)]:", kTotalProviders);

        for (size_t i = 0; i < resolved->m_container.size(); ++i) {
            auto& dst = resolved->m_container.at(i);
            std::string_view source;

            for (size_t pi = 0; pi < kTotalProviders; ++pi) {
                if (auto& cfg = providerConfigs[pi]; cfg && cfg->m_container.at(i).hasValue()) {
                    dst.copyValueFrom(cfg->m_container.at(i));
                    source = providerNames[pi];
                    break;
                }
            }

            if (!source.empty()) {
                LOG_I("  {:<{}} = {}  [{:<{}}]", dst.name(), maxParamLen, dst.valueToString(), source,
                      maxSourceLen);
            }
        }

        auto current = this->getConfig();
        if (!current || !current->m_container.equals(resolved->m_container)) {
            LOG_I("Config changed — publishing update.");
            this->setConfig(std::move(resolved));
        } else {
            LOG_I("Config unchanged — no update published.");
        }
    }

   private:
    std::tuple<std::unique_ptr<Providers>..., std::unique_ptr<DefaultProvider>> m_providers;
    Utils::Logging::Logger m_logger{"ConfigManager"};
};

}  // namespace Utils::Config
