#include <gtest/gtest.h>

#include <memory>
#include <sstream>

#include "Config/ConfigParser/JsonConfigParser.h"

// Plain struct for JSON parser tests — glaze-reflectable, no ConfigParameter fields.
struct PlainTestConfig {
    std::string name = "default";
    int value = 42;
    double rate = 3.14;
    bool enabled = true;
};

class testJsonConfigParser : public ::testing::Test {
   protected:
    void SetUp() override {
        testConfig.name = "json_test";
        testConfig.value = 123;
        testConfig.rate = 1.618;
        testConfig.enabled = true;
    }

    PlainTestConfig testConfig;
};

TEST_F(testJsonConfigParser, ParserCreation) {
    EXPECT_NO_THROW({ auto parser = std::make_unique<Utils::Config::ConfigParser::JsonConfigParser<PlainTestConfig>>(); });
}

TEST_F(testJsonConfigParser, ReadInvalidJson) {
    auto parser = std::make_unique<Utils::Config::ConfigParser::JsonConfigParser<PlainTestConfig>>();
    std::stringstream invalidJsonStream("{invalid json}");

    auto readConfig = parser->readConfig(invalidJsonStream);

    EXPECT_EQ(readConfig, nullptr);
}

TEST_F(testJsonConfigParser, ReadEmptyStream) {
    auto parser = std::make_unique<Utils::Config::ConfigParser::JsonConfigParser<PlainTestConfig>>();
    std::stringstream emptyStream("");

    auto readConfig = parser->readConfig(emptyStream);

    EXPECT_EQ(readConfig, nullptr);
}

TEST_F(testJsonConfigParser, WriteAndReadConfig) {
    auto parser = std::make_unique<Utils::Config::ConfigParser::JsonConfigParser<PlainTestConfig>>();
    std::stringstream stream;

    int writeResult = parser->writeConfig(testConfig, stream);
    EXPECT_EQ(writeResult, 0);

    std::string jsonContent = stream.str();
    EXPECT_FALSE(jsonContent.empty());
    EXPECT_NE(jsonContent.find("json_test"), std::string::npos);
    EXPECT_NE(jsonContent.find("123"), std::string::npos);

    std::stringstream readStream(jsonContent);
    auto readConfig = parser->readConfig(readStream);

    ASSERT_NE(readConfig, nullptr);
    EXPECT_EQ(readConfig->name, "json_test");
    EXPECT_EQ(readConfig->value, 123);
    EXPECT_DOUBLE_EQ(readConfig->rate, 1.618);
    EXPECT_TRUE(readConfig->enabled);
}

TEST_F(testJsonConfigParser, ReadPartialJson) {
    auto parser = std::make_unique<Utils::Config::ConfigParser::JsonConfigParser<PlainTestConfig>>();
    std::stringstream partialJsonStream(R"({"name": "partial", "value": 999})");

    auto readConfig = parser->readConfig(partialJsonStream);

    ASSERT_NE(readConfig, nullptr);
    EXPECT_EQ(readConfig->name, "partial");
    EXPECT_EQ(readConfig->value, 999);
    EXPECT_DOUBLE_EQ(readConfig->rate, 3.14);
    EXPECT_TRUE(readConfig->enabled);
}
