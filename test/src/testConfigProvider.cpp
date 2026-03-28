#include <gtest/gtest.h>

#include <string>

#include "Config/ConfigParameters/ConfigParameter.h"
#include "Config/ConfigParameters/ConfigParametersContainer.h"
#include "Config/Providers/DefaultConfigProvider.h"
#include "Mocks.h"

using namespace Utils::Config::ConfigParameters;
using Utils::Config::Providers::DefaultConfigProvider;

// ── ConfigParameter ──────────────────────────────────────────────────────────

class testConfigParameter : public ::testing::Test {
   protected:
    void SetUp() override {
        param = m_container.buildConfigParam<int>("port", "Server port", 8080,
                                                  [](int v) { return v > 0 && v < 65536; });
    }

    ConfigParametersContainer m_container;
    ConfigParameter<int> param{nullptr};
};

TEST_F(testConfigParameter, StartsWithNoValue) {
    EXPECT_FALSE(param.hasValue());
}

TEST_F(testConfigParameter, SetAndGet) {
    param.set(9090);
    ASSERT_TRUE(param.hasValue());
    EXPECT_EQ(param.get(), 9090);
}

TEST_F(testConfigParameter, Reset) {
    param.set(9090);
    param.reset();
    EXPECT_FALSE(param.hasValue());
}

TEST_F(testConfigParameter, ValidatorAcceptsValidValue) {
    param.set(443);
    EXPECT_TRUE(param.validate());
}

TEST_F(testConfigParameter, ValidatorRejectsInvalidValue) {
    param.set(-1);
    EXPECT_FALSE(param.validate());
}

TEST_F(testConfigParameter, ValidateReturnsTrueWhenUnset) {
    EXPECT_TRUE(param.validate());
}

// ── ConfigParametersContainer ─────────────────────────────────────────────────

class testConfigParametersContainer : public ::testing::Test {};

TEST_F(testConfigParametersContainer, ApplyDefaultsSetsValues) {
    ConfigParametersContainer container;
    auto port = container.buildConfigParam<int>("port", "Port", 8080);
    auto host = container.buildConfigParam<std::string>("host", "Host", std::string("localhost"));

    EXPECT_FALSE(port.hasValue());
    EXPECT_FALSE(host.hasValue());

    container.applyDefaults();

    ASSERT_TRUE(port.hasValue());
    EXPECT_EQ(port.get(), 8080);
    ASSERT_TRUE(host.hasValue());
    EXPECT_EQ(host.get(), "localhost");
}

TEST_F(testConfigParametersContainer, SizeReflectsRegisteredParams) {
    ConfigParametersContainer container;
    EXPECT_EQ(container.size(), 0u);
    container.buildConfigParam<int>("a", "a", 1);
    EXPECT_EQ(container.size(), 1u);
    container.buildConfigParam<int>("b", "b", 2);
    EXPECT_EQ(container.size(), 2u);
}

TEST_F(testConfigParametersContainer, EqualsReturnsTrueForIdenticalValues) {
    ConfigParametersContainer a;
    a.buildConfigParam<int>("port", "Port", 8080);
    a.applyDefaults();

    ConfigParametersContainer b;
    b.buildConfigParam<int>("port", "Port", 8080);
    b.applyDefaults();

    EXPECT_TRUE(a.equals(b));
}

TEST_F(testConfigParametersContainer, EqualsReturnsFalseForDifferentValues) {
    ConfigParametersContainer a;
    a.buildConfigParam<int>("port", "Port", 8080);
    a.applyDefaults();

    ConfigParametersContainer b;
    auto pb = b.buildConfigParam<int>("port", "Port", 8080);
    b.applyDefaults();
    pb.set(9090);

    EXPECT_FALSE(a.equals(b));
}

TEST_F(testConfigParametersContainer, CopyValueFromTransfersValue) {
    ConfigParametersContainer src;
    auto srcPort = src.buildConfigParam<int>("port", "Port", 8080);
    srcPort.set(9090);

    ConfigParametersContainer dst;
    dst.buildConfigParam<int>("port", "Port", 8080);

    dst.at(0).copyValueFrom(src.at(0));

    ASSERT_TRUE(dst.at(0).hasValue());
    EXPECT_EQ(dst.at(0).valueToString(), "9090");
}

// ── DefaultConfigProvider ────────────────────────────────────────────────────

class testDefaultConfigProvider : public ::testing::Test {};

TEST_F(testDefaultConfigProvider, CreatesConfigWithDefaultsApplied) {
    DefaultConfigProvider<TestConfig> provider;
    auto config = provider.getConfig();

    ASSERT_NE(config, nullptr);
    ASSERT_TRUE(config->name.hasValue());
    EXPECT_EQ(config->name.get(), "default_name");
    ASSERT_TRUE(config->value.hasValue());
    EXPECT_EQ(config->value.get(), 0);
    ASSERT_TRUE(config->enabled.hasValue());
    EXPECT_FALSE(config->enabled.get());
}

TEST_F(testDefaultConfigProvider, NameIsDefault) {
    DefaultConfigProvider<TestConfig> provider;
    EXPECT_EQ(provider.name(), "default");
}
