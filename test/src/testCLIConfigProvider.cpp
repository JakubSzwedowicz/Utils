#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Config/ConfigManager.h"
#include "Config/ConfigProviders/CLIConfigProvider.h"
#include "Mocks.h"

using namespace Utils::Config::ConfigProviders;
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

TEST(testCLIConfigProvider, ParsesEqualsFormat) {
    FakeArgv args{"prog", "--name=hello", "--value=42", "--rate=1.5", "--enabled=true"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

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

TEST(testCLIConfigProvider, ParsesSpaceSeparatedFormat) {
    FakeArgv args{"prog", "--name", "world", "--value", "7"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "world");
    ASSERT_TRUE(cfg->value.hasValue());
    EXPECT_EQ(cfg->value.get(), 7);
}

TEST(testCLIConfigProvider, BoolFlagWithNoValueIsTrue) {
    FakeArgv args{"prog", "--enabled"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->enabled.hasValue());
    EXPECT_TRUE(cfg->enabled.get());
}

TEST(testCLIConfigProvider, ExplicitFalseFlag) {
    FakeArgv args{"prog", "--enabled=false"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->enabled.hasValue());
    EXPECT_FALSE(cfg->enabled.get());
}

TEST(testCLIConfigProvider, HyphenNormalisedToUnderscore) {
    FakeArgv args{"prog", "--name=hyphen-test"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "hyphen-test");
}

TEST(testCLIConfigProvider, UnknownArgsAreIgnored) {
    FakeArgv args{"prog", "--unknown=xyz", "--name=known"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_EQ(cfg->name.get(), "known");
    EXPECT_FALSE(cfg->value.hasValue());
}

TEST(testCLIConfigProvider, UnparsedParamsHaveNoValue) {
    FakeArgv args{"prog", "--name=only"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_TRUE(cfg->name.hasValue());
    EXPECT_FALSE(cfg->value.hasValue());
    EXPECT_FALSE(cfg->rate.hasValue());
    EXPECT_FALSE(cfg->enabled.hasValue());
}

TEST(testCLIConfigProvider, NameIsCorrect) {
    FakeArgv args{"prog"};
    CLIConfigProvider<TestConfig> provider{args.argc(), args.argv()};
    EXPECT_EQ(provider.name(), "CLIConfigProvider");
}

// ── Inside ConfigManager ─────────────────────────────────────────────────────

TEST(testCLIConfigProviderInManager, CLIOverridesDefault) {
    FakeArgv args{"prog", "--name=from_cli", "--value=99"};
    ConfigManager<TestConfig, CLIConfigProvider<TestConfig>> manager{
        std::make_unique<CLIConfigProvider<TestConfig>>(args.argc(), args.argv())};

    auto cfg = manager.getConfig();
    ASSERT_NE(cfg, nullptr);
    EXPECT_EQ(cfg->name.get(), "from_cli");
    EXPECT_EQ(cfg->value.get(), 99);
    EXPECT_EQ(cfg->rate.get(), 0.0);
    EXPECT_FALSE(cfg->enabled.get());
}

// ── Tests for NestedConfig & LoggerConfig ────────────────────────────────────

TEST(testCLIConfigProviderNested, ParsesNestedLoggerConfig) {
    FakeArgv args{"prog", "--port=9090", "--loggerConfig={\"filename\":\"cli_test.log\",\"globalLogLevel\":2}"};
    CLIConfigProvider<NestedTestConfig> provider{args.argc(), args.argv()};

    auto cfg = provider.getConfig();
    ASSERT_NE(cfg, nullptr);
    ASSERT_TRUE(cfg->port.hasValue());
    EXPECT_EQ(cfg->port.get(), 9090);

    ASSERT_TRUE(cfg->loggerConfig.hasValue());
    EXPECT_EQ(cfg->loggerConfig.get().filename, "cli_test.log");
    EXPECT_EQ(cfg->loggerConfig.get().globalLogLevel, Utils::Logging::LogLevel::WARNING);
}
