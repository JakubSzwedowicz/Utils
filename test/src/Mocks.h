#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "Config/IConfigProvider.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

// Test configuration structure
struct TestConfig {
    std::string name = "default";
    int value = 42;
    double rate = 3.14;
    bool enabled = true;
};

// Mock subscriber for testing ConfigPublisher
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

// Helper function to create test config
inline std::shared_ptr<TestConfig> createTestConfig(const std::string& name = "test",
                                                   int value = 100,
                                                   double rate = 2.71,
                                                   bool enabled = false) {
    auto config = std::make_shared<TestConfig>();
    config->name = name;
    config->value = value;
    config->rate = rate;
    config->enabled = enabled;
    return config;
}
