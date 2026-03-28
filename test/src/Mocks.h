#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Config/ConfigParametersContainer.h"
#include "Config/ConfigParameter.h"
#include "Config/IConfigProvider.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

// ── Test config ──────────────────────────────────────────────────────────────

struct TestConfig {
    Utils::Config::ConfigParametersContainer m_container;

    Utils::Config::ConfigParameter<std::string> name =
        m_container.buildConfigParam<std::string>("name", "Test config name", std::string("default_name"));
    Utils::Config::ConfigParameter<int> value =
        m_container.buildConfigParam<int>("value", "Test config value", 0);
    Utils::Config::ConfigParameter<double> rate =
        m_container.buildConfigParam<double>("rate", "Test config rate", 0.0);
    Utils::Config::ConfigParameter<bool> enabled =
        m_container.buildConfigParam<bool>("enabled", "Test config enabled flag", false);

    TestConfig() = default;
    TestConfig(const TestConfig&) = delete;
    TestConfig& operator=(const TestConfig&) = delete;
};

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

class MockProvider1 : public Utils::Config::IConfigProvider<TestConfig> {
   public:
    void update(std::shared_ptr<TestConfig> config) { m_config = std::move(config); }
    std::shared_ptr<TestConfig> getConfig() const override { return m_config; }
    std::string_view name() const override { return "MockProvider1"; }

   private:
    std::shared_ptr<TestConfig> m_config;
};

class MockProvider2 : public Utils::Config::IConfigProvider<TestConfig> {
   public:
    void update(std::shared_ptr<TestConfig> config) { m_config = std::move(config); }
    std::shared_ptr<TestConfig> getConfig() const override { return m_config; }
    std::string_view name() const override { return "MockProvider2"; }

   private:
    std::shared_ptr<TestConfig> m_config;
};
