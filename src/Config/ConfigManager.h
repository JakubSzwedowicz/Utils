//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <array>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "ConfigPublisher.h"
#include "Logging/Logger.h"
#include "Logging/LoggerConfig.h"
#include "Logging/LoggerMacros.h"
#include "Providers/DefaultConfigProvider.h"
#include "Runnables/IRunnable.h"

namespace Utils::Config {

template <typename T>
concept HasLoggerConfig = requires(const T& t) { t.loggerConfig.get(); };

template <typename Config, typename... Providers>
class ConfigManager : public ConfigPublisher<Config>, public Runnables::IRunnable {
    using DefaultProvider = Utils::Config::Providers::DefaultConfigProvider<Config>;
    static constexpr size_t kTotalProviders = sizeof...(Providers) + 1;

   public:
    ConfigManager()
        requires(std::default_initializable<Providers> && ...)
        : m_providers(std::make_unique<Providers>()..., std::make_unique<DefaultProvider>()) {
        resolve();
    }

    // Only available when pack is non-empty to avoid ambiguity with the default constructor.
    explicit ConfigManager(std::unique_ptr<Providers>... providers)
        requires(sizeof...(Providers) > 0)
        : m_providers(std::move(providers)..., std::make_unique<DefaultProvider>()) {
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
        getProvider<T>().poll();  // drain so run() doesn't re-resolve redundantly
    }

    void run() override {
        bool anyChanged = false;
        std::apply([&](auto&... ptrs) { ((ptrs->run(), anyChanged |= ptrs->poll().has_value()), ...); }, m_providers);
        if (anyChanged) resolve();
    }

    void setConfig(std::shared_ptr<const Config> config) override {
        ConfigPublisher<Config>::setConfig(config);
        if constexpr (HasLoggerConfig<Config>) {
            if (config) {
                auto lc = std::make_shared<Logging::LoggerConfig>(config->loggerConfig.get());
                auto current = m_loggerConfigPublisher.getConfig();
                if (!current || *current != *lc) m_loggerConfigPublisher.setConfig(lc);
            }
        }
    }

    std::shared_ptr<const Logging::LoggerConfig> getLoggerConfig() const { return m_loggerConfigPublisher.getConfig(); }

    void addLogSink(std::shared_ptr<spdlog::sinks::sink> sink) { m_logger.addSink(sink); }

    void resolve() {
        auto resolved = std::make_shared<Config>();

        auto providerConfigs = std::apply(
            [](auto&... p) { return std::array<std::shared_ptr<const Config>, sizeof...(p)>{p->getConfig()...}; },
            m_providers);

        auto providerNames = std::apply(
            [](auto&... p) { return std::array<std::string_view, sizeof...(p)>{p->name()...}; }, m_providers);

        size_t maxParamLen = 0;
        for (size_t i = 0; i < resolved->m_container.size(); ++i)
            maxParamLen = std::max(maxParamLen, resolved->m_container.at(i).name().size());

        size_t maxSourceLen = 0;
        for (auto sv : providerNames) maxSourceLen = std::max(maxSourceLen, sv.size());

        struct ParamLog {
            std::string_view name;
            std::string value;
            std::string_view source;
        };
        std::vector<ParamLog> paramLogs;

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
            if (!source.empty()) paramLogs.push_back({dst.name(), dst.valueToString(), source});
        }

        auto current = ConfigPublisher<Config>::getConfig();
        const bool changed = !current || !current->m_container.equals(resolved->m_container);
        if (changed) setConfig(resolved);

        LOG_I("Resolving config [{} provider(s)]:", kTotalProviders);
        for (auto& [name, value, source] : paramLogs)
            LOG_I("  {:<{}} = {}  [{:<{}}]", name, maxParamLen, value, source, maxSourceLen);
        LOG_I("{}", changed ? "Config changed — publishing update." : "Config unchanged — no update published.");
    }

   private:
    std::tuple<std::unique_ptr<Providers>..., std::unique_ptr<DefaultProvider>> m_providers;
    ConfigPublisher<Logging::LoggerConfig> m_loggerConfigPublisher;
    Utils::Logging::Logger m_logger{"ConfigManager"};
};

}  // namespace Utils::Config
