#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Config/ConfigManager.h"
#include "Logging/LoggerConfig.h"
#include "Mocks.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

using namespace Utils::Config;
using namespace Utils::Logging;

// ── Subscriber helpers ────────────────────────────────────────────────────────

class LoggerConfigSubscriber : public Utils::PublishSubscribe::ISubscriber<std::shared_ptr<const LoggerConfig>> {
   public:
    void onUpdate(const std::shared_ptr<const LoggerConfig>& config) override {
        last = config;
        count++;
    }
    std::shared_ptr<const LoggerConfig> last;
    int count{0};
};

// ── Mock provider for NestedTestConfig ────────────────────────────────────────

class MockNestedProvider : public Utils::Config::ConfigProviders::IConfigProvider<NestedTestConfig> {
   public:
    void update(std::shared_ptr<NestedTestConfig> config) {
        m_config = std::move(config);
        m_hasNew = true;
    }
    void run() override {}
    std::optional<std::shared_ptr<NestedTestConfig>> poll() override {
        if (m_hasNew) {
            m_hasNew = false;
            return m_config;
        }
        return std::nullopt;
    }
    std::shared_ptr<const NestedTestConfig> getConfig() const override { return m_config; }
    std::string_view name() const override { return "MockNestedProvider"; }

   private:
    std::shared_ptr<NestedTestConfig> m_config;
    bool m_hasNew{false};
};

// ── Core ConfigManager tests ──────────────────────────────────────────────────

class testConfigManager : public ::testing::Test {
   protected:
    void TearDown() override {
        auto* mgr = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const TestConfig>>::getManager();
        if (subscriber) mgr->removeSubscriber(subscriber.get());
    }
    std::shared_ptr<MockConfigSubscriber> subscriber;
};

TEST_F(testConfigManager, InitialConfigHasDefaults) {
    ConfigManager<TestConfig> manager;
    auto config = manager.getConfig();

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "default_name");
    EXPECT_EQ(config->value.get(), 0);
    EXPECT_DOUBLE_EQ(config->rate.get(), 0.0);
    EXPECT_FALSE(config->enabled.get());
}

TEST_F(testConfigManager, ProviderOverridesDefault) {
    ConfigManager<TestConfig, MockProvider1> manager{std::make_unique<MockProvider1>()};
    manager.getProvider<MockProvider1>().update(createTestConfig("from_provider", 42, 1.5, true));
    manager.run();

    auto config = manager.getConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "from_provider");
    EXPECT_EQ(config->value.get(), 42);
    EXPECT_DOUBLE_EQ(config->rate.get(), 1.5);
    EXPECT_TRUE(config->enabled.get());
}

TEST_F(testConfigManager, FirstProviderHasHighestPriority) {
    ConfigManager<TestConfig, MockProvider1, MockProvider2> manager{std::make_unique<MockProvider1>(),
                                                                    std::make_unique<MockProvider2>()};
    manager.getProvider<MockProvider1>().update(createTestConfig("from_p1", 1));
    manager.getProvider<MockProvider2>().update(createTestConfig("from_p2", 2));
    manager.run();

    auto config = manager.getConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "from_p1");
    EXPECT_EQ(config->value.get(), 1);
}

TEST_F(testConfigManager, SecondProviderFillsWhatFirstDoesNotSet) {
    ConfigManager<TestConfig, MockProvider1, MockProvider2> manager{std::make_unique<MockProvider1>(),
                                                                    std::make_unique<MockProvider2>()};

    auto p1Config = std::make_shared<TestConfig>();
    p1Config->name.set("from_p1");

    auto p2Config = std::make_shared<TestConfig>();
    p2Config->value.set(99);

    manager.getProvider<MockProvider1>().update(p1Config);
    manager.getProvider<MockProvider2>().update(p2Config);
    manager.run();

    auto config = manager.getConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "from_p1");
    EXPECT_EQ(config->value.get(), 99);
    EXPECT_DOUBLE_EQ(config->rate.get(), 0.0);
}

TEST_F(testConfigManager, PublishesOnConfigChange) {
    ConfigManager<TestConfig, MockProvider1> manager{std::make_unique<MockProvider1>()};
    subscriber = std::make_shared<MockConfigSubscriber>();
    Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const TestConfig>>::getManager()->addSubscriber(
        subscriber.get());
    int countBefore = subscriber->updateCount;

    manager.getProvider<MockProvider1>().update(createTestConfig("changed", 999));
    manager.run();

    EXPECT_GT(subscriber->updateCount, countBefore);
    ASSERT_NE(subscriber->lastReceivedConfig, nullptr);
    EXPECT_EQ(subscriber->lastReceivedConfig->name.get(), "changed");
}

TEST_F(testConfigManager, DoesNotPublishWhenConfigUnchanged) {
    ConfigManager<TestConfig, MockProvider1> manager{std::make_unique<MockProvider1>()};
    subscriber = std::make_shared<MockConfigSubscriber>();
    Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const TestConfig>>::getManager()->addSubscriber(
        subscriber.get());

    manager.getProvider<MockProvider1>().update(createTestConfig("same", 1));
    manager.run();
    int countAfterFirst = subscriber->updateCount;

    manager.getProvider<MockProvider1>().update(createTestConfig("same", 1));
    manager.run();
    EXPECT_EQ(subscriber->updateCount, countAfterFirst);
}

// ── Logger config integration tests (HasLoggerConfig path) ───────────────────

class testConfigManagerLogger : public ::testing::Test {
   protected:
    void TearDown() override {
        auto* mgr = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const LoggerConfig>>::getManager();
        if (m_sub) mgr->removeSubscriber(m_sub.get());
    }
    std::shared_ptr<LoggerConfigSubscriber> m_sub;
};

TEST_F(testConfigManagerLogger, LoggerConfigPublishedOnConstruction) {
    ConfigManager<NestedTestConfig> manager;
    auto lc = manager.getLoggerConfig();
    ASSERT_NE(lc, nullptr);
    EXPECT_EQ(lc->globalLogLevel, LogLevel::INFO);
}

TEST_F(testConfigManagerLogger, LoggerConfigUpdatesWithProvider) {
    ConfigManager<NestedTestConfig, MockNestedProvider> manager{std::make_unique<MockNestedProvider>()};

    auto cfg = std::make_shared<NestedTestConfig>();
    LoggerConfig lc;
    lc.globalLogLevel = LogLevel::DEBUG;
    lc.filename = "debug.log";
    cfg->loggerConfig.set(lc);
    manager.getProvider<MockNestedProvider>().update(std::move(cfg));
    manager.run();

    auto published = manager.getLoggerConfig();
    ASSERT_NE(published, nullptr);
    EXPECT_EQ(published->globalLogLevel, LogLevel::DEBUG);
    EXPECT_EQ(published->filename, "debug.log");
}

TEST_F(testConfigManagerLogger, LoggerConfigNotRepublishedWhenUnchanged) {
    ConfigManager<NestedTestConfig, MockNestedProvider> manager{std::make_unique<MockNestedProvider>()};
    m_sub = std::make_shared<LoggerConfigSubscriber>();
    Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const LoggerConfig>>::getManager()->addSubscriber(
        m_sub.get());
    int countAfterConstruction = m_sub->count;

    auto cfg = std::make_shared<NestedTestConfig>();
    cfg->loggerConfig.set(LoggerConfig{});  // same as default
    manager.getProvider<MockNestedProvider>().update(std::move(cfg));
    manager.run();

    EXPECT_EQ(m_sub->count, countAfterConstruction);
}

TEST_F(testConfigManagerLogger, LoggerConfigRepublishedOnChange) {
    ConfigManager<NestedTestConfig, MockNestedProvider> manager{std::make_unique<MockNestedProvider>()};
    m_sub = std::make_shared<LoggerConfigSubscriber>();
    Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const LoggerConfig>>::getManager()->addSubscriber(
        m_sub.get());
    int countAfterConstruction = m_sub->count;

    LoggerConfig changed;
    changed.globalLogLevel = LogLevel::WARNING;
    auto cfg = std::make_shared<NestedTestConfig>();
    cfg->loggerConfig.set(changed);
    manager.getProvider<MockNestedProvider>().update(std::move(cfg));
    manager.run();

    EXPECT_GT(m_sub->count, countAfterConstruction);
    ASSERT_NE(m_sub->last, nullptr);
    EXPECT_EQ(m_sub->last->globalLogLevel, LogLevel::WARNING);
}

TEST_F(testConfigManagerLogger, ConfigWithNoLoggerConfigDoesNotPublishLoggerConfig) {
    // TestConfig has no loggerConfig — the LoggerConfig pub/sub channel must stay silent.
    m_sub = std::make_shared<LoggerConfigSubscriber>();
    Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const LoggerConfig>>::getManager()->addSubscriber(
        m_sub.get());

    ConfigManager<TestConfig, MockProvider1> manager{std::make_unique<MockProvider1>()};
    manager.getProvider<MockProvider1>().update(createTestConfig("x", 1));
    manager.run();

    EXPECT_EQ(m_sub->count, 0);
}
