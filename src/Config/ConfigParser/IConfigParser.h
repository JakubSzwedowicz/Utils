//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once
#include <memory>

namespace Utils::Config::ConfigParser {

template <typename Config, typename Source>
class IConfigParser {
   public:
    virtual ~IConfigParser() = default;

    virtual std::shared_ptr<Config> readConfig(Source source) const = 0;
};

}  // namespace Utils::Config::ConfigParser
