#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <csignal>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Utils::Logging {

namespace {

class LoggerContext {
   public:
    static LoggerContext& instance() {
        static LoggerContext ctx;
        return ctx;
    }

    void add(const std::string& name, Logger* logger) {
        std::lock_guard lock(m_registryMutex);
        m_registry[name] = logger;
    }

    void remove(const std::string& name) {
        std::lock_guard lock(m_registryMutex);
        m_registry.erase(name);
    }

    Logger* find(const std::string& name) const {
        std::lock_guard lock(m_registryMutex);
        auto it = m_registry.find(name);
        return it != m_registry.end() ? it->second : nullptr;
    }

    std::pair<std::shared_ptr<spdlog::sinks::stdout_color_sink_mt>, std::shared_ptr<spdlog::sinks::basic_file_sink_mt>>
    getSinks(const std::string& filename) {
        std::lock_guard lock(m_sinksMutex);
        if (!m_consoleSink) {
            m_consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            m_consoleSink->set_level(spdlog::level::trace);
        }
        if (!m_fileSink || m_currentFilename != filename) {
            m_fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            m_fileSink->set_level(spdlog::level::trace);
            m_currentFilename = filename;
        }
        return {m_consoleSink, m_fileSink};
    }

   private:
    LoggerContext() = default;

    mutable std::mutex m_registryMutex;
    std::unordered_map<std::string, Logger*> m_registry;

    std::mutex m_sinksMutex;
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_consoleSink;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_fileSink;
    std::string m_currentFilename;
};

}  // namespace

constexpr spdlog::level::level_enum logLevelToSpdlogImpl(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return spdlog::level::debug;
        case LogLevel::INFO:
            return spdlog::level::info;
        case LogLevel::WARNING:
            return spdlog::level::warn;
        case LogLevel::ERROR:
            return spdlog::level::err;
        case LogLevel::CRITICAL:
            return spdlog::level::critical;
        case LogLevel::OFF:
            return spdlog::level::off;
        default:
            return spdlog::level::info;
    }
}

consteval spdlog::level::level_enum logLevelToSpdlog(LogLevel level) { return logLevelToSpdlogImpl(level); }

Logger::Logger(std::string name) : m_name(std::move(name)) {
    LoggerContext::instance().add(m_name, this);
    pull();
    // Fallback: no config published yet — build with defaults.
    {
        std::unique_lock lk(m_loggerMutex);
        if (!m_logger) m_logger = buildLogger(m_name, m_config, {}, true);
    }
}

Logger::~Logger() { LoggerContext::instance().remove(m_name); }

Logger& Logger::getInstance() {
    static Logger instance("Root");
    return instance;
}

Logger* Logger::find(const std::string& name) { return LoggerContext::instance().find(name); }

const std::string& Logger::getName() const { return m_name; }

void Logger::onUpdate(const std::shared_ptr<const LoggerConfig>& newConfig) {
    if (!newConfig) return;

    bool filenameChanged;
    {
        std::lock_guard lock(m_configMutex);
        filenameChanged = newConfig->filename != m_config->filename;
    }

    if (filenameChanged) {
        // Warm up the global file sink for the new filename before rebuilding
        // so concurrent onUpdate calls from other loggers are safe — only the
        // first one creates (and truncates) the file sink.
        LoggerContext::instance().getSinks(newConfig->filename);
        rebuildLogger(newConfig);
    } else {
        {
            std::lock_guard lock(m_configMutex);
            m_config = newConfig;
        }
        updateLoggerLevel();
    }
}

// Hot path — fully lock-free. m_logger is atomically loaded; the resulting
// shared_ptr keeps the spdlog::logger alive even if another thread calls
// rebuildLogger() and swaps it out mid-flight.
template <LogLevel Level>
void Logger::log(const char* file, int line, const char* func, std::string_view message) {
    spdlog::source_loc loc{file, line, func};
    std::shared_lock lk(m_loggerMutex);
    m_logger->log(loc, logLevelToSpdlog(Level), message);
}

void Logger::flush() {
    std::shared_lock lk(m_loggerMutex);
    m_logger->flush();
}

void Logger::addSink(std::shared_ptr<spdlog::sinks::sink> sink) {
    if (!sink) return;
    sink->set_level(spdlog::level::trace);

    std::shared_ptr<const LoggerConfig> cfg;
    std::vector<std::shared_ptr<spdlog::sinks::sink>> extras;
    bool useStd;
    {
        std::lock_guard lock(m_configMutex);
        m_extraSinks.push_back(sink);
        cfg = m_config;
        extras = m_extraSinks;
        useStd = m_useStandardSinks;
    }
    {
        std::unique_lock lk(m_loggerMutex);
        m_logger = buildLogger(m_name, cfg, extras, useStd);
    }
}

void Logger::clearSinks() {
    std::shared_ptr<const LoggerConfig> cfg;
    {
        std::lock_guard lock(m_configMutex);
        m_extraSinks.clear();
        m_useStandardSinks = false;
        cfg = m_config;
    }
    {
        std::unique_lock lk(m_loggerMutex);
        m_logger = buildLogger(m_name, cfg, {}, false);
    }
}

void Logger::updateLoggerLevel() {
    std::shared_ptr<const LoggerConfig> cfg;
    {
        std::lock_guard lock(m_configMutex);
        cfg = m_config;
    }
    LogLevel threshold = cfg->globalLogLevel;
    if (const auto it = cfg->loggersLogLevels.find(m_name); it != cfg->loggersLogLevels.end()) threshold = it->second;
    std::shared_lock lk(m_loggerMutex);
    m_logger->set_level(logLevelToSpdlogImpl(threshold));
}

void Logger::rebuildLogger(const std::shared_ptr<const LoggerConfig>& config) {
    std::shared_ptr<const LoggerConfig> cfg;
    std::vector<std::shared_ptr<spdlog::sinks::sink>> extras;
    bool useStd;
    {
        std::lock_guard lock(m_configMutex);
        m_config = config;
        cfg = config;
        extras = m_extraSinks;
        useStd = m_useStandardSinks;
    }

    {
        std::shared_lock lk(m_loggerMutex);
        if (m_logger) m_logger->flush();
    }
    {
        std::unique_lock lk(m_loggerMutex);
        m_logger = buildLogger(m_name, cfg, extras, useStd);
    }

    // Level update reads m_config (just set above) and calls set_level on the
    // newly stored spdlog logger.
    updateLoggerLevel();
}

std::shared_ptr<spdlog::logger> Logger::buildLogger(const std::string& name,
                                                    const std::shared_ptr<const LoggerConfig>& config,
                                                    const std::vector<std::shared_ptr<spdlog::sinks::sink>>& extraSinks,
                                                    bool useStandardSinks) {
    struct LifecycleManager {
        LifecycleManager() {
            std::atexit([]() { spdlog::shutdown(); });
            auto handler = [](int sig) {
                spdlog::shutdown();
                std::signal(sig, SIG_DFL);
                std::raise(sig);
            };
            std::signal(SIGINT, handler);
            std::signal(SIGTERM, handler);
        }
    };
    static LifecycleManager s_lifecycleManager;

    std::vector<spdlog::sink_ptr> sinks;

    if (useStandardSinks) {
        auto [consoleSink, fileSink] = LoggerContext::instance().getSinks(config->filename);
        sinks = {consoleSink, fileSink};
    }

    for (const auto& s : extraSinks) sinks.push_back(s);

    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [P%P:T%t] [%^%l%$] [%s:%#] [%n::%!] %v");
    logger->flush_on(spdlog::level::warn);
    return logger;
}

// ── Explicit instantiations ───────────────────────────────────────────────────

template void Logger::log<LogLevel::DEBUG>(const char*, int, const char*, std::string_view);
template void Logger::log<LogLevel::INFO>(const char*, int, const char*, std::string_view);
template void Logger::log<LogLevel::WARNING>(const char*, int, const char*, std::string_view);
template void Logger::log<LogLevel::ERROR>(const char*, int, const char*, std::string_view);
template void Logger::log<LogLevel::CRITICAL>(const char*, int, const char*, std::string_view);
template void Logger::log<LogLevel::OFF>(const char*, int, const char*, std::string_view);

}  // namespace Utils::Logging
