#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "Config/ConfigManager.h"
#include "Config/Providers/JsonConfigProvider.h"
#include "Mocks.h"

using namespace Utils::Config::Providers;
using Utils::Config::ConfigManager;

// ── JsonConfigProvider<TestConfig> ───────────────────────────────────────────

class testJsonConfigProvider : public ::testing::Test {
   protected:
    JsonConfigProvider<TestConfig> provider;
};

TEST_F(testJsonConfigProvider, NameIsCorrect) { EXPECT_EQ(provider.name(), "JsonConfigProvider"); }

TEST_F(testJsonConfigProvider, InitialGetConfigReturnsNull) { EXPECT_EQ(provider.getConfig(), nullptr); }

TEST_F(testJsonConfigProvider, ParsesAllFlatFields) {
    std::stringstream ss(R"({"name":"json_test","value":42,"rate":1.5,"enabled":true})");
    provider.update(ss);

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "json_test");
    ASSERT_TRUE(cfg->value.hasValue());
    EXPECT_EQ(cfg->value.get(), 42);
    ASSERT_TRUE(cfg->rate.hasValue());
    EXPECT_DOUBLE_EQ(cfg->rate.get(), 1.5);
    ASSERT_TRUE(cfg->enabled.hasValue());
    EXPECT_TRUE(cfg->enabled.get());
}

TEST_F(testJsonConfigProvider, InvalidJsonReturnsNullConfig) {
    std::stringstream ss("{not valid json}");
    provider.update(ss);

    EXPECT_EQ(provider.getConfig(), nullptr);
}

TEST_F(testJsonConfigProvider, EmptyStreamReturnsNullConfig) {
    std::stringstream ss("");
    provider.update(ss);

    EXPECT_EQ(provider.getConfig(), nullptr);
}

TEST_F(testJsonConfigProvider, PartialJsonOnlySetsSpecifiedFields) {
    std::stringstream ss(R"({"name":"partial","value":99})");
    provider.update(ss);

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "partial");
    ASSERT_TRUE(cfg->value.hasValue());
    EXPECT_EQ(cfg->value.get(), 99);
    // Fields absent from JSON are not set
    EXPECT_FALSE(cfg->rate.hasValue());
    EXPECT_FALSE(cfg->enabled.hasValue());
}

// ── JsonConfigProvider<NestedTestConfig> — LoggerConfig embedding ─────────────

class testJsonConfigProviderNested : public ::testing::Test {
   protected:
    JsonConfigProvider<NestedTestConfig> provider;
};

TEST_F(testJsonConfigProviderNested, ParsesNestedLoggerConfig) {
    std::stringstream ss(R"({"port":9090,"loggerConfig":{"filename":"json_test.log","globalLogLevel":2}})");
    provider.update(ss);

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->port.hasValue());
    EXPECT_EQ(cfg->port.get(), 9090);

    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    EXPECT_EQ(cfg->loggerConfig.get().filename, "json_test.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::WARNING);
}

TEST_F(testJsonConfigProviderNested, PartialLoggerConfigUsesStructDefaults) {
    // Only filename supplied — globalLogLevel stays at its struct default (INFO)
    std::stringstream ss(R"({"port":8080,"loggerConfig":{"filename":"partial.log"}})");
    provider.update(ss);

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    EXPECT_EQ(cfg->loggerConfig.get().filename, "partial.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::INFO);
}

TEST_F(testJsonConfigProviderNested, LoggersLogLevelsParsed) {
    std::stringstream ss(R"({
        "port": 8080,
        "loggerConfig": {
            "filename": "lvl.log",
            "globalLogLevel": 1,
            "loggersLogLevels": { "db": 3, "net": 0 }
        }
    })");
    provider.update(ss);

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    const auto& lc = cfg->loggerConfig.get();
    EXPECT_EQ(lc.loggersLogLevels.at("db"), Utils::Logging::LogLevel::ERROR);
    EXPECT_EQ(lc.loggersLogLevels.at("net"), Utils::Logging::LogLevel::DEBUG);
}

// ── Inside ConfigManager ──────────────────────────────────────────────────────

TEST(testJsonConfigProviderInManager, JsonOverridesDefault) {
    ConfigManager<NestedTestConfig, JsonConfigProvider<NestedTestConfig>> manager;

    std::stringstream ss(R"({"port":7777,"loggerConfig":{"filename":"mgr.log","globalLogLevel":0}})");
    manager.update<JsonConfigProvider<NestedTestConfig>>(ss);

    auto cfg = manager.getConfig();
    ASSERT_NE(cfg, nullptr);
    EXPECT_EQ(cfg->port.get(), 7777);
    EXPECT_EQ(cfg->loggerConfig.get().filename, "mgr.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::DEBUG);
}

TEST(testJsonConfigProviderInManager, UnsetFieldsFallBackToDefault) {
    ConfigManager<NestedTestConfig, JsonConfigProvider<NestedTestConfig>> manager;

    // Only port is in JSON — loggerConfig falls back to DefaultConfigProvider value
    std::stringstream ss(R"({"port":1234})");
    manager.update<JsonConfigProvider<NestedTestConfig>>(ss);

    auto cfg = manager.getConfig();
    ASSERT_NE(cfg, nullptr);
    EXPECT_EQ(cfg->port.get(), 1234);
    // Default LoggerConfig has filename="mainLog.txt" and globalLogLevel=INFO
    EXPECT_EQ(cfg->loggerConfig.get().filename, "mainLog.txt");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::INFO);
}
