//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <string_view>

#include "Providers/IResourceProvider.h"

namespace Utils::Config::Providers {

// getConfig() returns the last parsed config without draining the poll() flag.
template <typename Config>
class IConfigProvider : public Utils::Providers::IResourceProvider<std::shared_ptr<Config>> {
   public:
    virtual std::shared_ptr<const Config> getConfig() const = 0;
    virtual std::string_view name() const = 0;
};

}  // namespace Utils::Config::Providers
