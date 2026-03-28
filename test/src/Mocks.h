#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Config/ConfigParameters/ConfigParameter.h"
#include "Config/ConfigParameters/ConfigParametersContainer.h"
#include "Config/Providers/IConfigProvider.h"
#include "Logging/LoggerConfig.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

// ── Test config ──────────────────────────────────────────────────────────────

struct TestConfig {
    Utils::Config::ConfigParameters::ConfigParametersContainer m_container;

    Utils::Config::ConfigParameters::ConfigParameter<std::string> name =
        m_container.buildConfigParam<std::string>("name", "Test config name", std::string("default_name"));
    Utils::Config::ConfigParameters::ConfigParameter<int> value =
        m_container.buildConfigParam<int>("value", "Test config value", 0);
    Utils::Config::ConfigParameters::ConfigParameter<double> rate =
        m_container.buildConfigParam<double>("rate", "Test config rate", 0.0);
    Utils::Config::ConfigParameters::ConfigParameter<bool> enabled =
        m_container.buildConfigParam<bool>("enabled", "Test config enabled flag", false);

    TestConfig() = default;
    TestConfig(const TestConfig&) = delete;
    TestConfig& operator=(const TestConfig&) = delete;
};

struct NestedTestConfig {
    Utils::Config::ConfigParameters::ConfigParametersContainer m_container;

    Utils::Config::ConfigParameters::ConfigParameter<int> port =
        m_container.buildConfigParam<int>("port", "Test port", 8080);

    Utils::Config::ConfigParameters::ConfigParameter<Utils::Logging::LoggerConfig> loggerConfig =
        m_container.buildConfigParam<Utils::Logging::LoggerConfig>("loggerConfig", "Logger config",
                                                                   Utils::Logging::LoggerConfig{});

    NestedTestConfig() = default;
    NestedTestConfig(const NestedTestConfig&) = delete;
    NestedTestConfig& operator=(const NestedTestConfig&) = delete;
};

#include "glaze/glaze.hpp"

namespace glz {
template <>
struct meta<TestConfig> {
    using T = TestConfig;
    static constexpr auto value =
        object("name", &T::name, "value", &T::value, "rate", &T::rate, "enabled", &T::enabled);
};

template <>
struct meta<NestedTestConfig> {
    using T = NestedTestConfig;
    static constexpr auto value = object("port", &T::port, "loggerConfig", &T::loggerConfig);
};
}  // namespace glz

// Helper: creates a TestConfig with explicit values set on each parameter.
// Does NOT call applyDefaults() — values are explicitly set.
inline std::shared_ptr<TestConfig> createTestConfig(const std::string& name = "test", int value = 100,
                                                    double rate = 2.71, bool enabled = false) {
    auto config = std::make_shared<TestConfig>();
    config->name.set(name);
    config->value.set(value);
    config->rate.set(rate);
    config->enabled.set(enabled);
    return config;
}

// ── Pub/Sub mock ─────────────────────────────────────────────────────────────

class MockConfigSubscriber : public Utils::PublishSubscribe::ISubscriber<std::shared_ptr<TestConfig>> {
   public:
    MockConfigSubscriber() : ISubscriber<std::shared_ptr<TestConfig>>() {}

    void onUpdate(const std::shared_ptr<TestConfig>& config) override {
        lastReceivedConfig = config;
        updateCount++;
    }

   public:
    std::shared_ptr<TestConfig> lastReceivedConfig = nullptr;
    int updateCount = 0;
};

// ── Mock providers for ConfigManager tests ───────────────────────────────────

class MockProvider1 : public Utils::Config::Providers::IConfigProvider<TestConfig> {
   public:
    void update(std::shared_ptr<TestConfig> config) { m_config = std::move(config); }
    std::shared_ptr<TestConfig> getConfig() const override { return m_config; }
    std::string_view name() const override { return "MockProvider1"; }

   private:
    std::shared_ptr<TestConfig> m_config;
};

class MockProvider2 : public Utils::Config::Providers::IConfigProvider<TestConfig> {
   public:
    void update(std::shared_ptr<TestConfig> config) { m_config = std::move(config); }
    std::shared_ptr<TestConfig> getConfig() const override { return m_config; }
    std::string_view name() const override { return "MockProvider2"; }

   private:
    std::shared_ptr<TestConfig> m_config;
};
