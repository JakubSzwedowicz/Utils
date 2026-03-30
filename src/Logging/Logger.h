#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>

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
    explicit Logger(std::string name, std::shared_ptr<const LoggerConfig> config = nullptr);

    static Logger& getInstance();

    const std::string& getName() const;

    void onUpdate(const std::shared_ptr<const LoggerConfig>& newConfig) override;

    template <LogLevel Level>
    void log(const char* file, int line, const char* func, std::string_view message);

    void flush();

    void addSink(std::shared_ptr<spdlog::sinks::sink> sink);

    void clearSinks();

   private:
    void updateLoggerLevel();

    static std::shared_ptr<spdlog::logger> buildLogger(const std::string& name,
                                                       const std::shared_ptr<const LoggerConfig>& config);

   private:
    std::string m_name;
    mutable std::mutex m_mutex;
    std::shared_ptr<const LoggerConfig> m_config = std::make_shared<LoggerConfig>();

    const std::shared_ptr<spdlog::logger> m_logger;
};

}  // namespace Utils::Logging
