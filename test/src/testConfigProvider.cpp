#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

#include "Config/IConfigProvider.h"
#include "Mocks.h"

// Test IConfigProvider functionality
class testConfigProvider : public ::testing::Test {
protected:
    void SetUp() override {
        provider = std::make_unique<Utils::Config::IConfigProvider<TestConfig>>();
        testConfig = createTestConfig("test", 100, 2.71, false);
    }

    std::unique_ptr<Utils::Config::IConfigProvider<TestConfig>> provider;
    std::shared_ptr<TestConfig> testConfig;
};

TEST_F(testConfigProvider, SetAndGetConfig) {
    // Initially should have no config
    EXPECT_EQ(provider->getConfig(), nullptr);

    // Set config and verify it's stored
    provider->setConfig(testConfig);
    auto retrievedConfig = provider->getConfig();

    ASSERT_NE(retrievedConfig, nullptr);
    EXPECT_EQ(retrievedConfig->name, "test");
    EXPECT_EQ(retrievedConfig->value, 100);
    EXPECT_DOUBLE_EQ(retrievedConfig->rate, 2.71);
    EXPECT_FALSE(retrievedConfig->enabled);
}

TEST_F(testConfigProvider, ThreadSafety) {
    const int numThreads = 10;
    const int numOperations = 100;
    std::atomic<int> successCount{0};

    // Function to set and get config in a loop
    auto workerFunction = [&](int threadId) {
        for (int i = 0; i < numOperations; ++i) {
            auto config = createTestConfig("thread_" + std::to_string(threadId),
                                         threadId * 1000 + i,
                                         1.41,
                                         true);

            provider->setConfig(config);

            auto retrieved = provider->getConfig();
            if (retrieved != nullptr) {
                successCount.fetch_add(1);
            }

            // Small delay to increase chance of race conditions
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    };

    // Launch multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerFunction, i);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify we got successful operations and final config is valid
    EXPECT_GT(successCount.load(), 0);
    auto finalConfig = provider->getConfig();
    EXPECT_NE(finalConfig, nullptr);
}

TEST_F(testConfigProvider, ConfigReplacement) {
    // Set initial config
    provider->setConfig(testConfig);

    // Create and set a new config
    auto newConfig = createTestConfig("replaced", 999, 3.33, true);
    provider->setConfig(newConfig);

    // Verify the config was replaced
    auto retrievedConfig = provider->getConfig();
    ASSERT_NE(retrievedConfig, nullptr);
    EXPECT_EQ(retrievedConfig->name, "replaced");
    EXPECT_EQ(retrievedConfig->value, 999);
    EXPECT_DOUBLE_EQ(retrievedConfig->rate, 3.33);
    EXPECT_TRUE(retrievedConfig->enabled);
}
