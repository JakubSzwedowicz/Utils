#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <csignal>
#include <mutex>
#include <string>

namespace Utils::Logging {

// ── Global shared sinks ───────────────────────────────────────────────────────

namespace {

struct GlobalSinks {
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> consoleSink;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink;
    std::string currentFilename;
    std::mutex mutex;
};

GlobalSinks& globalSinks() {
    static GlobalSinks s;
    return s;
}

// Must be called with globalSinks().mutex held.
void ensureFileSink_locked(GlobalSinks& gs, const std::string& filename) {
    if (gs.fileSink && gs.currentFilename == filename) return;
    gs.fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
    gs.fileSink->set_level(spdlog::level::trace);
    gs.currentFilename = filename;
}

}  // namespace

// ── Static member definitions ─────────────────────────────────────────────────

std::unordered_map<std::string, Logger*> Logger::s_registry;
std::mutex Logger::s_registryMutex;

// ── Helpers ───────────────────────────────────────────────────────────────────

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

// ── Logger implementation ─────────────────────────────────────────────────────

Logger::Logger(std::string name, std::shared_ptr<const LoggerConfig> config)
    : m_name(std::move(name)),
      m_config(config ? config : std::make_shared<LoggerConfig>()),
      m_logger(buildLogger(m_name, m_config)) {
    updateLoggerLevel();
    std::lock_guard lock(s_registryMutex);
    s_registry[m_name] = this;
}

Logger::~Logger() {
    std::lock_guard lock(s_registryMutex);
    s_registry.erase(m_name);
}

Logger& Logger::getInstance() {
    static Logger instance("Root");
    return instance;
}

Logger* Logger::find(const std::string& name) {
    std::lock_guard lock(s_registryMutex);
    auto it = s_registry.find(name);
    return it != s_registry.end() ? it->second : nullptr;
}

const std::string& Logger::getName() const { return m_name; }

void Logger::onUpdate(const std::shared_ptr<const LoggerConfig>& newConfig) {
    if (!newConfig) return;

    const bool filenameChanged = newConfig->filename != m_config->filename;

    {
        std::lock_guard lock(m_mutex);
        m_config = newConfig;
    }

    if (filenameChanged) {
        {
            std::lock_guard gsLock(globalSinks().mutex);
            ensureFileSink_locked(globalSinks(), newConfig->filename);
        }
        // Rebuild every live logger so they all share the new file sink.
        std::lock_guard regLock(s_registryMutex);
        for (auto& [name, logger] : s_registry) logger->rebuildLogger();
    } else {
        updateLoggerLevel();
    }
}

template <LogLevel Level>
void Logger::log(const char* file, int line, const char* func, std::string_view message) {
    spdlog::source_loc loc{file, line, func};
    m_logger->log(loc, logLevelToSpdlog(Level), message);
}

void Logger::flush() { m_logger->flush(); }

void Logger::addSink(std::shared_ptr<spdlog::sinks::sink> sink) {
    if (!sink) return;
    m_logger->sinks().push_back(sink);
    sink->set_level(spdlog::level::trace);
}

void Logger::clearSinks() { m_logger->sinks().clear(); }

void Logger::updateLoggerLevel() {
    std::lock_guard lock(m_mutex);
    LogLevel threshold = m_config->globalLogLevel;
    if (const auto it = m_config->loggersLogLevels.find(m_name); it != m_config->loggersLogLevels.end())
        threshold = it->second;
    m_logger->set_level(logLevelToSpdlogImpl(threshold));
}

void Logger::rebuildLogger() {
    std::lock_guard lock(m_mutex);
    m_logger = buildLogger(m_name, m_config);
    // Inline level update to avoid re-acquiring m_mutex.
    LogLevel threshold = m_config->globalLogLevel;
    if (const auto it = m_config->loggersLogLevels.find(m_name); it != m_config->loggersLogLevels.end())
        threshold = it->second;
    m_logger->set_level(logLevelToSpdlogImpl(threshold));
}

std::shared_ptr<spdlog::logger> Logger::buildLogger(const std::string& name,
                                                    const std::shared_ptr<const LoggerConfig>& config) {
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

    auto& gs = globalSinks();
    std::lock_guard gsLock(gs.mutex);

    if (!gs.consoleSink) {
        gs.consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        gs.consoleSink->set_level(spdlog::level::trace);
    }
    ensureFileSink_locked(gs, config->filename);

    auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{gs.consoleSink, gs.fileSink});
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
