#include <gtest/gtest.h>
#include <memory>
#include <sstream>

#include "Config/ConfigParser/JsonConfigParser.h"
#include "Mocks.h"

// Note: Since glaze reflection is commented out, these tests will focus on
// the interface and error handling rather than actual JSON serialization
class testJsonConfigParser : public ::testing::Test {
protected:
    void SetUp() override {
        // Note: JsonConfigParser requires glaze reflection to be properly set up
        // For now, we'll test the interface and error conditions
        testConfig.name = "json_test";
        testConfig.value = 123;
        testConfig.rate = 1.618;
        testConfig.enabled = true;
    }

    TestConfig testConfig;
    // Note: We'll create the parser in each test method due to potential
    // glaze reflection requirements
};

TEST_F(testJsonConfigParser, ParserCreation) {
    // Test that parser can be created (this will fail if glaze reflection isn't set up)
    EXPECT_NO_THROW({
        auto parser = std::make_unique<Utils::Config::JsonConfigParser<TestConfig>>();
    });
}

TEST_F(testJsonConfigParser, ReadInvalidJson) {
    auto parser = std::make_unique<Utils::Config::JsonConfigParser<TestConfig>>();
    std::stringstream invalidJsonStream("{invalid json}");

    auto readConfig = parser->readConfig(invalidJsonStream);

    // Should return nullptr for invalid JSON
    EXPECT_EQ(readConfig, nullptr);
}

TEST_F(testJsonConfigParser, ReadEmptyStream) {
    auto parser = std::make_unique<Utils::Config::JsonConfigParser<TestConfig>>();
    std::stringstream emptyStream("");

    auto readConfig = parser->readConfig(emptyStream);

    // Should return nullptr for empty stream
    EXPECT_EQ(readConfig, nullptr);
}

// Note: The following tests would require proper glaze reflection setup
// which is currently commented out in the original test file


TEST_F(testJsonConfigParser, WriteAndReadConfig) {
    auto parser = std::make_unique<Utils::Config::JsonConfigParser<TestConfig>>();
    std::stringstream stream;

    // Write config to JSON
    int writeResult = parser->writeConfig(testConfig, stream);
    EXPECT_EQ(writeResult, 0); // Success

    // Verify JSON was written
    std::string jsonContent = stream.str();
    EXPECT_FALSE(jsonContent.empty());
    EXPECT_NE(jsonContent.find("json_test"), std::string::npos);
    EXPECT_NE(jsonContent.find("123"), std::string::npos);

    // Read config back from JSON
    std::stringstream readStream(jsonContent);
    auto readConfig = parser->readConfig(readStream);

    // Verify config was read correctly
    ASSERT_NE(readConfig, nullptr);
    EXPECT_EQ(readConfig->name, "json_test");
    EXPECT_EQ(readConfig->value, 123);
    EXPECT_DOUBLE_EQ(readConfig->rate, 1.618);
    EXPECT_TRUE(readConfig->enabled);
}

TEST_F(testJsonConfigParser, ReadPartialJson) {
    auto parser = std::make_unique<Utils::Config::JsonConfigParser<TestConfig>>();
    // JSON with only some fields
    std::stringstream partialJsonStream(R"({"name": "partial", "value": 999})");

    auto readConfig = parser->readConfig(partialJsonStream);

    // Should read successfully with default values for missing fields
    ASSERT_NE(readConfig, nullptr);
    EXPECT_EQ(readConfig->name, "partial");
    EXPECT_EQ(readConfig->value, 999);
    // Should have default values for missing fields
    EXPECT_DOUBLE_EQ(readConfig->rate, 3.14); // default value
    EXPECT_TRUE(readConfig->enabled); // default value
}
