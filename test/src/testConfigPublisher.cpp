#include <gtest/gtest.h>

#include <memory>

#include "Config/ConfigPublisher.h"
#include "Mocks.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

class testConfigPublisher : public ::testing::Test {
   protected:
    void SetUp() override {
        publisher = std::make_unique<Utils::Config::ConfigPublisher<TestConfig>>();
        subscriber = std::make_shared<MockConfigSubscriber>();
        testConfig = createTestConfig("publisher_test", 200, 1.23, true);
    }

    void TearDown() override {
        auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
        manager->removeSubscriber(subscriber.get());
    }

    std::unique_ptr<Utils::Config::ConfigPublisher<TestConfig>> publisher;
    std::shared_ptr<MockConfigSubscriber> subscriber;
    std::shared_ptr<TestConfig> testConfig;
};

TEST_F(testConfigPublisher, SetConfigTriggersPublish) {
    auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
    manager->addSubscriber(subscriber.get());

    EXPECT_EQ(subscriber->updateCount, 0);

    publisher->setConfig(testConfig);

    EXPECT_EQ(subscriber->updateCount, 1);
    ASSERT_NE(subscriber->lastReceivedConfig, nullptr);
    EXPECT_EQ(subscriber->lastReceivedConfig->name.get(), "publisher_test");
    EXPECT_EQ(subscriber->lastReceivedConfig->value.get(), 200);

    auto retrieved = publisher->getConfig();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->name.get(), "publisher_test");
}

TEST_F(testConfigPublisher, MultipleConfigUpdates) {
    auto manager = Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<TestConfig>>::getManager();
    manager->addSubscriber(subscriber.get());

    for (int i = 0; i < 5; ++i) {
        publisher->setConfig(createTestConfig("config_" + std::to_string(i), i * 10, 2.5 + i, i % 2 == 0));
    }

    EXPECT_EQ(subscriber->updateCount, 5);
    ASSERT_NE(subscriber->lastReceivedConfig, nullptr);
    EXPECT_EQ(subscriber->lastReceivedConfig->name.get(), "config_4");
    EXPECT_EQ(subscriber->lastReceivedConfig->value.get(), 40);
}
