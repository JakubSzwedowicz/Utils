#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Config/ConfigManager.h"
#include "Config/Providers/JsonConfigProvider.h"
#include "Mocks.h"

using namespace Utils::Config::Providers;
using Utils::Config::ConfigManager;

// ── JsonConfigProvider<TestConfig> ───────────────────────────────────────────
//
// Each fixture wires a StringSourceProvider into the provider under test.
// push(json) + run() replaces the old update(stream) API.

class testJsonConfigProvider : public ::testing::Test {
   protected:
    void SetUp() override {
        auto src = std::make_unique<StringSourceProvider>();
        m_src = src.get();
        m_provider = std::make_unique<JsonConfigProvider<TestConfig>>(std::move(src));
    }

    StringSourceProvider* m_src{nullptr};
    std::unique_ptr<JsonConfigProvider<TestConfig>> m_provider;
};

TEST_F(testJsonConfigProvider, NameIsCorrect) { EXPECT_EQ(m_provider->name(), "JsonConfigProvider"); }

TEST_F(testJsonConfigProvider, InitialGetConfigReturnsNull) { EXPECT_EQ(m_provider->getConfig(), nullptr); }

TEST_F(testJsonConfigProvider, ParsesAllFlatFields) {
    m_src->push(R"({"name":"json_test","value":42,"rate":1.5,"enabled":true})");
    m_provider->run();

    auto cfg = m_provider->getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->name.hasValue());    EXPECT_EQ(cfg->name.get(),        "json_test");
    ASSERT_TRUE(cfg->value.hasValue());   EXPECT_EQ(cfg->value.get(),       42);
    ASSERT_TRUE(cfg->rate.hasValue());    EXPECT_DOUBLE_EQ(cfg->rate.get(), 1.5);
    ASSERT_TRUE(cfg->enabled.hasValue()); EXPECT_TRUE(cfg->enabled.get());
}

TEST_F(testJsonConfigProvider, InvalidJsonReturnsNullConfig) {
    m_src->push("{not valid json}");
    m_provider->run();

    EXPECT_EQ(m_provider->getConfig(), nullptr);
}

TEST_F(testJsonConfigProvider, EmptyStringReturnsNullConfig) {
    m_src->push("");
    m_provider->run();

    EXPECT_EQ(m_provider->getConfig(), nullptr);
}

TEST_F(testJsonConfigProvider, PartialJsonOnlySetsSpecifiedFields) {
    m_src->push(R"({"name":"partial","value":99})");
    m_provider->run();

    auto cfg = m_provider->getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->name.hasValue());  EXPECT_EQ(cfg->name.get(),  "partial");
    ASSERT_TRUE(cfg->value.hasValue()); EXPECT_EQ(cfg->value.get(), 99);
    // Fields absent from JSON are not set
    EXPECT_FALSE(cfg->rate.hasValue());
    EXPECT_FALSE(cfg->enabled.hasValue());
}

TEST_F(testJsonConfigProvider, PollReturnsTrueAfterRun) {
    m_src->push(R"({"name":"poll_test","value":1})");
    m_provider->run();

    EXPECT_TRUE(m_provider->poll().has_value());
    // Second poll with no new run() returns nullopt
    EXPECT_FALSE(m_provider->poll().has_value());
}

// ── JsonConfigProvider<NestedTestConfig> — LoggerConfig embedding ─────────────

class testJsonConfigProviderNested : public ::testing::Test {
   protected:
    void SetUp() override {
        auto src = std::make_unique<StringSourceProvider>();
        m_src = src.get();
        m_provider = std::make_unique<JsonConfigProvider<NestedTestConfig>>(std::move(src));
    }

    StringSourceProvider* m_src{nullptr};
    std::unique_ptr<JsonConfigProvider<NestedTestConfig>> m_provider;
};

TEST_F(testJsonConfigProviderNested, ParsesNestedLoggerConfig) {
    m_src->push(R"({"port":9090,"loggerConfig":{"filename":"json_test.log","globalLogLevel":2}})");
    m_provider->run();

    auto cfg = m_provider->getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->port.hasValue());
    EXPECT_EQ(cfg->port.get(), 9090);

    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    EXPECT_EQ(cfg->loggerConfig.get().filename,       "json_test.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::WARNING);
}

TEST_F(testJsonConfigProviderNested, PartialLoggerConfigUsesStructDefaults) {
    // Only filename supplied — globalLogLevel stays at its struct default (INFO)
    m_src->push(R"({"port":8080,"loggerConfig":{"filename":"partial.log"}})");
    m_provider->run();

    auto cfg = m_provider->getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    EXPECT_EQ(cfg->loggerConfig.get().filename,       "partial.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::INFO);
}

TEST_F(testJsonConfigProviderNested, LoggersLogLevelsParsed) {
    m_src->push(R"({
        "port": 8080,
        "loggerConfig": {
            "filename": "lvl.log",
            "globalLogLevel": 1,
            "loggersLogLevels": { "db": 3, "net": 0 }
        }
    })");
    m_provider->run();

    auto cfg = m_provider->getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    const auto& lc = cfg->loggerConfig.get();
    EXPECT_EQ(lc.loggersLogLevels.at("db"),  Utils::Logging::LogLevel::ERROR);
    EXPECT_EQ(lc.loggersLogLevels.at("net"), Utils::Logging::LogLevel::DEBUG);
}

// ── Inside ConfigManager ──────────────────────────────────────────────────────

TEST(testJsonConfigProviderInManager, JsonOverridesDefault) {
    auto source = std::make_unique<StringSourceProvider>();
    StringSourceProvider* src = source.get();
    ConfigManager<NestedTestConfig, JsonConfigProvider<NestedTestConfig>> manager(
        std::make_unique<JsonConfigProvider<NestedTestConfig>>(std::move(source)));

    src->push(R"({"port":7777,"loggerConfig":{"filename":"mgr.log","globalLogLevel":0}})");
    manager.run();

    auto cfg = manager.getConfig();
    ASSERT_NE(cfg, nullptr);
    EXPECT_EQ(cfg->port.get(),                        7777);
    EXPECT_EQ(cfg->loggerConfig.get().filename,       "mgr.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::DEBUG);
}

TEST(testJsonConfigProviderInManager, UnsetFieldsFallBackToDefault) {
    auto source = std::make_unique<StringSourceProvider>();
    StringSourceProvider* src = source.get();
    ConfigManager<NestedTestConfig, JsonConfigProvider<NestedTestConfig>> manager(
        std::make_unique<JsonConfigProvider<NestedTestConfig>>(std::move(source)));

    // Only port is in JSON — loggerConfig falls back to DefaultConfigProvider value
    src->push(R"({"port":1234})");
    manager.run();

    auto cfg = manager.getConfig();
    ASSERT_NE(cfg, nullptr);
    EXPECT_EQ(cfg->port.get(), 1234);
    // Default LoggerConfig has filename="mainLog.txt" and globalLogLevel=INFO
    EXPECT_EQ(cfg->loggerConfig.get().filename,       "mainLog.txt");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::INFO);
}
