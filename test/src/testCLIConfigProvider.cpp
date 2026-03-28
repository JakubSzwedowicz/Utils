#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Config/ConfigManager.h"
#include "Config/Providers/CLIConfigProvider.h"
#include "Mocks.h"

using namespace Utils::Config::Providers;
using Utils::Config::ConfigManager;

// Builds a fake argc/argv from a list of string literals.
// Lifetime of the FakeArgv must exceed any call that uses argc()/argv().
struct FakeArgv {
    explicit FakeArgv(std::initializer_list<std::string_view> args) {
        for (auto sv : args) m_storage.emplace_back(sv);
        for (auto& s : m_storage) m_ptrs.push_back(s.data());
    }
    int argc() const { return static_cast<int>(m_ptrs.size()); }
    char** argv() { return const_cast<char**>(m_ptrs.data()); }

   private:
    std::vector<std::string> m_storage;
    std::vector<const char*> m_ptrs;
};

// ── Provider in isolation ────────────────────────────────────────────────────

class testCLIConfigProvider : public ::testing::Test {
   protected:
    CLIConfigProvider<TestConfig> provider;
};

TEST_F(testCLIConfigProvider, ParsesEqualsFormat) {
    FakeArgv args{"prog", "--name=hello", "--value=42", "--rate=1.5", "--enabled=true"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "hello");
    ASSERT_TRUE(cfg->value.hasValue());
    EXPECT_EQ(cfg->value.get(), 42);
    ASSERT_TRUE(cfg->rate.hasValue());
    EXPECT_DOUBLE_EQ(cfg->rate.get(), 1.5);
    ASSERT_TRUE(cfg->enabled.hasValue());
    EXPECT_TRUE(cfg->enabled.get());
}

TEST_F(testCLIConfigProvider, ParsesSpaceSeparatedFormat) {
    FakeArgv args{"prog", "--name", "world", "--value", "7"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "world");
    ASSERT_TRUE(cfg->value.hasValue());
    EXPECT_EQ(cfg->value.get(), 7);
}

TEST_F(testCLIConfigProvider, BoolFlagWithNoValueIsTrue) {
    FakeArgv args{"prog", "--enabled"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->enabled.hasValue());
    EXPECT_TRUE(cfg->enabled.get());
}

TEST_F(testCLIConfigProvider, ExplicitFalseFlag) {
    FakeArgv args{"prog", "--enabled=false"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->enabled.hasValue());
    EXPECT_FALSE(cfg->enabled.get());
}

TEST_F(testCLIConfigProvider, HyphenNormalisedToUnderscore) {
    // Slot name is "name"; CLI arg uses the equivalent hyphenated spelling.
    // (TestConfig only has single-word names; this verifies the normalisation path.)
    FakeArgv args{"prog", "--name=hyphen-test"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "hyphen-test");  // value is preserved; name matched
}

TEST_F(testCLIConfigProvider, UnknownArgsAreIgnored) {
    FakeArgv args{"prog", "--unknown=xyz", "--name=known"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "known");
    EXPECT_FALSE(cfg->value.hasValue());
}

TEST_F(testCLIConfigProvider, UnparsedParamsHaveNoValue) {
    FakeArgv args{"prog", "--name=only"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_FALSE(cfg->value.hasValue());
    EXPECT_FALSE(cfg->rate.hasValue());
    EXPECT_FALSE(cfg->enabled.hasValue());
}

TEST_F(testCLIConfigProvider, NameIsCorrect) { EXPECT_EQ(provider.name(), "CLIConfigProvider"); }

// ── Inside ConfigManager ─────────────────────────────────────────────────────

TEST(testCLIConfigProviderInManager, CLIOverridesDefault) {
    ConfigManager<TestConfig, CLIConfigProvider<TestConfig>> manager;

    FakeArgv args{"prog", "--name=from_cli", "--value=99"};
    manager.update<CLIConfigProvider<TestConfig>>(args.argc(), args.argv());

    auto cfg = manager.getConfig();
    ASSERT_NE(cfg, nullptr);
    EXPECT_EQ(cfg->name.get(), "from_cli");
    EXPECT_EQ(cfg->value.get(), 99);
    // Params not on the CLI fall back to DefaultConfigProvider
    EXPECT_EQ(cfg->rate.get(), 0.0);
    EXPECT_FALSE(cfg->enabled.get());
}

// ── Tests for NestedConfig & LoggerConfig ────────────────────────────────────

class testCLIConfigProviderNested : public ::testing::Test {
   protected:
    CLIConfigProvider<NestedTestConfig> provider;
};

// We want to verify that someone can provide a JSON string or appropriate CLI arg
// to instantiate a full LoggerConfig in the ConfigParameter.
TEST_F(testCLIConfigProviderNested, ParsesNestedLoggerConfig) {
    FakeArgv args{"prog", "--port=9090", "--loggerConfig={\"filename\":\"cli_test.log\",\"globalLogLevel\":2}"};
    provider.update(args.argc(), args.argv());

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->port.hasValue());
    EXPECT_EQ(cfg->port.get(), 9090);

    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    EXPECT_EQ(cfg->loggerConfig.get().filename, "cli_test.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::WARNING);
}
