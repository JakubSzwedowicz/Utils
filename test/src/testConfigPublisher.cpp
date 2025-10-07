#include <gtest/gtest.h>
#include <memory>

#include "Config/ConfigManagers.h"
#include "PublishSubscribe/IPublisherSubscriber.h"
#include "Mocks.h"

// Test ConfigPublisher functionality
class testConfigPublisher : public ::testing::Test {
protected:
    void SetUp() override {
        publisher = std::make_unique<Utils::Config::ConfigPublisher<TestConfig>>();
        subscriber = std::make_shared<MockConfigSubscriber>();
        testConfig = createTestConfig("publisher_test", 200, 1.23, true);
    }

    void TearDown() override {
        // Clean up any remaining subscribers
        auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
        manager->removeSubscriber(subscriber.get());
    }

    std::unique_ptr<Utils::Config::ConfigPublisher<TestConfig>> publisher;
    std::shared_ptr<MockConfigSubscriber> subscriber;
    std::shared_ptr<TestConfig> testConfig;
};

TEST_F(testConfigPublisher, SetConfigTriggersPublish) {
    // Register subscriber
    auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
    manager->addSubscriber(subscriber.get());

    // Initially no updates
    EXPECT_EQ(subscriber->updateCount, 0);

    // Set config should trigger publish
    publisher->setConfig(testConfig);

    // Verify subscriber received the update
    EXPECT_EQ(subscriber->updateCount, 1);
    ASSERT_NE(subscriber->lastReceivedConfig, nullptr);
    EXPECT_EQ(subscriber->lastReceivedConfig->name, "publisher_test");
    EXPECT_EQ(subscriber->lastReceivedConfig->value, 200);

    // Verify config is also stored in provider
    auto retrievedConfig = publisher->getConfig();
    ASSERT_NE(retrievedConfig, nullptr);
    EXPECT_EQ(retrievedConfig->name, "publisher_test");
}

TEST_F(testConfigPublisher, MultipleConfigUpdates) {
    auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
    manager->addSubscriber(subscriber.get());

    // Set multiple configs
    for (int i = 0; i < 5; ++i) {
        auto config = createTestConfig("config_" + std::to_string(i), i * 10, 2.5 + i, i % 2 == 0);
        publisher->setConfig(config);
    }

    // Verify all updates were received
    EXPECT_EQ(subscriber->updateCount, 5);
    ASSERT_NE(subscriber->lastReceivedConfig, nullptr);
    EXPECT_EQ(subscriber->lastReceivedConfig->name, "config_4");
    EXPECT_EQ(subscriber->lastReceivedConfig->value, 40);
}
