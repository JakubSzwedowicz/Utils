#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>

#include "Logging/Logger.h"
#include "Logging/LoggerMacros.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

using namespace Utils::Logging;

template <typename Mutex>
class TestSink : public spdlog::sinks::base_sink<Mutex> {
   public:
    std::string log_contents;

   protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        log_contents += fmt::to_string(formatted);
    }
    void flush_() override {}
};

using TestSink_mt = TestSink<std::mutex>;

class LoggerTest : public ::testing::Test {
   protected:
    LoggerTest() : m_logger("TestLogger"), testSink(std::make_shared<TestSink_mt>()) {
        m_logger.clearSinks();
        m_logger.addSink(testSink);
        testSink->set_level(spdlog::level::trace);
        // Ensure a known starting level regardless of global pub/sub state.
        auto config = std::make_shared<LoggerConfig>();
        config->globalLogLevel = LogLevel::INFO;
        m_logger.onUpdate(config);
    }

    Logger m_logger;
    std::shared_ptr<TestSink_mt> testSink;
};

TEST_F(LoggerTest, LevelFilteringAtInfo) {
    LOG_D("Debug message");
    LOG_I("Info message");
    LOG_W("Warning message");

    EXPECT_FALSE(testSink->log_contents.find("Debug message") != std::string::npos);
    EXPECT_TRUE(testSink->log_contents.find("Info message") != std::string::npos);
    EXPECT_TRUE(testSink->log_contents.find("Warning message") != std::string::npos);
}

TEST_F(LoggerTest, FormattingWorks) {
    LOG_I("String: {}, Int: {}, Float: {:.2f}", "test", 42, 3.14159);
    EXPECT_TRUE(testSink->log_contents.find("String: test, Int: 42, Float: 3.14") != std::string::npos);
}

TEST_F(LoggerTest, GlobalLevelChangeEnablesDebug) {
    LOG_D("Before");
    EXPECT_FALSE(testSink->log_contents.find("Before") != std::string::npos);

    auto cfg = std::make_shared<LoggerConfig>();
    cfg->globalLogLevel = LogLevel::DEBUG;
    m_logger.onUpdate(cfg);

    LOG_D("After");
    EXPECT_TRUE(testSink->log_contents.find("After") != std::string::npos);
}

TEST_F(LoggerTest, ReceivesLogLevelUpdateViaPubSub) {
    LOG_D("before publish");
    EXPECT_FALSE(testSink->log_contents.find("before publish") != std::string::npos);

    auto cfg = std::make_shared<LoggerConfig>();
    cfg->globalLogLevel = LogLevel::DEBUG;
    Utils::PublishSubscribe::PublishSubscribeManager<std::shared_ptr<const LoggerConfig>>::getManager()
        ->publishMessage(cfg);

    LOG_D("after publish");
    EXPECT_TRUE(testSink->log_contents.find("after publish") != std::string::npos);
}

TEST_F(LoggerTest, PerLoggerOverrideEnablesDebug) {
    auto cfg = std::make_shared<LoggerConfig>();
    cfg->globalLogLevel = LogLevel::INFO;
    cfg->loggersLogLevels[m_logger.getName()] = LogLevel::DEBUG;
    m_logger.onUpdate(cfg);

    LOG_D("Debug via per-logger override");
    EXPECT_TRUE(testSink->log_contents.find("Debug via per-logger override") != std::string::npos);
}
