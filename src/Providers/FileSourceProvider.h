//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "ISourceProvider.h"
#include "Logging/Logger.h"

namespace Utils::Providers {

class FileSourceProvider : public ISourceProvider<std::string> {
   public:
    explicit FileSourceProvider(std::filesystem::path path);

    void run() override;
    std::optional<std::string> poll() override;

   private:
    std::filesystem::path m_path;
    std::filesystem::file_time_type m_lastMtime{};
    std::optional<std::string> m_pending;
    Utils::Logging::Logger m_logger{"FileSourceProvider"};
};

}  // namespace Utils::Providers
