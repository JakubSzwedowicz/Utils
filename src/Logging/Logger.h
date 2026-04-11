#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include "LoggerConfig.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

namespace spdlog {
class logger;
namespace sinks {
class sink;
}
}  // namespace spdlog

namespace Utils::Logging {

class Logger : public PublishSubscribe::ISubscriber<std::shared_ptr<const LoggerConfig>> {
   public:
    explicit Logger(std::string name);
    ~Logger() override;

    template <LogLevel Level>
    void log(const char* file, int line, const char* func, std::string_view message);
    void onUpdate(const std::shared_ptr<const LoggerConfig>& newConfig) override;

    void flush();
    void addSink(std::shared_ptr<spdlog::sinks::sink> sink);
    // Remove all extra sinks and suppress the standard console+file sinks.
    // Intended for tests that want to capture output exclusively.
    void clearSinks();

    static Logger* find(const std::string& name);
    const std::string& getName() const;
    static Logger& getInstance();

   private:
    void rebuildLogger(const std::shared_ptr<const LoggerConfig>& config);
    void updateLoggerLevel();

    // useStandardSinks=true  → {consoleSink, fileSink} + extraSinks
    // useStandardSinks=false → extraSinks only  (used after clearSinks())
    static std::shared_ptr<spdlog::logger> buildLogger(
        const std::string& name, const std::shared_ptr<const LoggerConfig>& config,
        const std::vector<std::shared_ptr<spdlog::sinks::sink>>& extraSinks, bool useStandardSinks);

   private:
    std::string m_name;

    mutable std::mutex m_configMutex;
    std::shared_ptr<const LoggerConfig> m_config = std::make_shared<LoggerConfig>();
    std::vector<std::shared_ptr<spdlog::sinks::sink>> m_extraSinks;
    bool m_useStandardSinks = true;

    mutable std::shared_mutex m_loggerMutex;
    std::shared_ptr<spdlog::logger> m_logger;
};

}  // namespace Utils::Logging
