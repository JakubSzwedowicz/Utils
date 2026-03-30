#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Config/ConfigManager.h"
#include "Logging/Logger.h"
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

// ── Resolve log column alignment test ────────────────────────────────────────

template <typename Mutex>
class CapturingSink : public spdlog::sinks::base_sink<Mutex> {
   public:
    std::vector<std::string> lines;

   protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t buf;
        this->formatter_->format(msg, buf);
        std::string s = fmt::to_string(buf);
        if (!s.empty() && s.back() == '\n') s.pop_back();
        lines.push_back(std::move(s));
    }
    void flush_() override {}
};
using CapturingSink_mt = CapturingSink<std::mutex>;

TEST(testConfigManagerResolveLog, ValueColumnIsAligned) {
    // Use two providers so params come from different sources (different source-name lengths).
    ConfigManager<TestConfig, MockProvider1, MockProvider2> manager{std::make_unique<MockProvider1>(),
                                                                    std::make_unique<MockProvider2>()};

    auto sink = std::make_shared<CapturingSink_mt>();
    Logger::find("ConfigManager")->addSink(sink);

    // Give params different value lengths: short int vs long string vs bool.
    auto p1 = std::make_shared<TestConfig>();
    p1->name.set("a_very_long_name_value");
    p1->value.set(1);

    auto p2 = std::make_shared<TestConfig>();
    p2->rate.set(3.14);
    p2->enabled.set(true);

    manager.getProvider<MockProvider1>().update(p1);
    manager.getProvider<MockProvider2>().update(p2);
    manager.run();

    // Collect all param lines (they contain " = ").
    std::vector<std::string> paramLines;
    for (const auto& line : sink->lines)
        if (line.find(" = ") != std::string::npos) paramLines.push_back(line);

    ASSERT_GE(paramLines.size(), 2u);

    // The source bracket "  [" must start at the same column in every param line.
    auto bracketCol = [](const std::string& line) -> size_t {
        // Find the last "  [" sequence (the source column).
        auto pos = line.rfind("  [");
        return pos == std::string::npos ? 0 : pos;
    };

    size_t expected = bracketCol(paramLines.front());
    for (const auto& line : paramLines) EXPECT_EQ(bracketCol(line), expected) << "misaligned line: " << line;
}
