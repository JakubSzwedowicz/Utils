//
// Created by Jakub Szwedowicz on 3/29/26.
//

#include "FileSourceProvider.h"

#include <fstream>
#include <iterator>
#include <system_error>

#include "Logging/LoggerMacros.h"

namespace Utils::Providers {

FileSourceProvider::FileSourceProvider(std::filesystem::path path) : m_path(std::move(path)) {}

void FileSourceProvider::setPath(std::filesystem::path path) {
    m_path = std::move(path);
    m_lastMtime = {};
}

void FileSourceProvider::run() {
    std::error_code ec;
    const auto mtime = std::filesystem::last_write_time(m_path, ec);
    if (ec) {
        LOG_W("FileSourceProvider: cannot stat '{}': {}", m_path.string(), ec.message());
        return;
    }
    if (mtime == m_lastMtime) return;
    m_lastMtime = mtime;

    std::ifstream file(m_path);
    if (!file.is_open()) {
        LOG_W("FileSourceProvider: cannot open '{}'", m_path.string());
        return;
    }
    m_pending = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{});
}

std::optional<std::string> FileSourceProvider::poll() { return std::exchange(m_pending, std::nullopt); }

}  // namespace Utils::Providers
