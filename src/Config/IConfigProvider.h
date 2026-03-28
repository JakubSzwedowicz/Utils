//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <string_view>

namespace Utils::Config {

template <typename Config>
class IConfigProvider {
   public:
    virtual ~IConfigProvider() = default;
    virtual std::shared_ptr<Config> getConfig() const = 0;
    virtual std::string_view name() const = 0;
};

}  // namespace Utils::Config
