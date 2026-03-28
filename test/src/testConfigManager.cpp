#include <gtest/gtest.h>

#include <memory>

#include "Config/ConfigManager.h"
#include "Mocks.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

using namespace Utils::Config;

class testConfigManager : public ::testing::Test {
   protected:
    void TearDown() override {
        auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
        if (subscriber) manager->removeSubscriber(subscriber.get());
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
    ConfigManager<TestConfig, MockProvider1> manager;

    manager.update<MockProvider1>(createTestConfig("from_provider", 42, 1.5, true));

    auto config = manager.getConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "from_provider");
    EXPECT_EQ(config->value.get(), 42);
    EXPECT_DOUBLE_EQ(config->rate.get(), 1.5);
    EXPECT_TRUE(config->enabled.get());
}

TEST_F(testConfigManager, FirstProviderHasHighestPriority) {
    ConfigManager<TestConfig, MockProvider1, MockProvider2> manager;

    // Both providers set values — MockProvider1 (first = highest priority) should win
    manager.update<MockProvider1>(createTestConfig("from_p1", 1));
    manager.update<MockProvider2>(createTestConfig("from_p2", 2));

    auto config = manager.getConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "from_p1");
    EXPECT_EQ(config->value.get(), 1);
}

TEST_F(testConfigManager, SecondProviderFillsWhatFirstDoesNotSet) {
    ConfigManager<TestConfig, MockProvider1, MockProvider2> manager;

    // MockProvider1 sets only name; MockProvider2 sets only value.
    // After resolve: name from p1 (higher priority), value from p2 (only setter).
    auto p1Config = std::make_shared<TestConfig>();
    p1Config->name.set("from_p1");

    auto p2Config = std::make_shared<TestConfig>();
    p2Config->value.set(99);

    manager.update<MockProvider1>(p1Config);
    manager.update<MockProvider2>(p2Config);

    auto config = manager.getConfig();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->name.get(), "from_p1");
    EXPECT_EQ(config->value.get(), 99);
    // Fields not set by any provider fall back to DefaultConfigProvider
    EXPECT_EQ(config->rate.get(), 0.0);
}

TEST_F(testConfigManager, PublishesOnConfigChange) {
    ConfigManager<TestConfig, MockProvider1> manager;
    subscriber = std::make_shared<MockConfigSubscriber>();

    auto psManager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
    psManager->addSubscriber(subscriber.get());

    // Initial resolve() in constructor already published once — reset the count
    int countBeforeUpdate = subscriber->updateCount;

    manager.update<MockProvider1>(createTestConfig("changed", 999));

    EXPECT_GT(subscriber->updateCount, countBeforeUpdate);
    ASSERT_NE(subscriber->lastReceivedConfig, nullptr);
    EXPECT_EQ(subscriber->lastReceivedConfig->name.get(), "changed");
}

TEST_F(testConfigManager, DoesNotPublishWhenConfigUnchanged) {
    ConfigManager<TestConfig, MockProvider1> manager;
    subscriber = std::make_shared<MockConfigSubscriber>();

    auto psManager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
    psManager->addSubscriber(subscriber.get());

    auto cfg = createTestConfig("same", 1);
    manager.update<MockProvider1>(cfg);
    int countAfterFirst = subscriber->updateCount;

    // Same values — should not publish again
    manager.update<MockProvider1>(createTestConfig("same", 1));
    EXPECT_EQ(subscriber->updateCount, countAfterFirst);
}
