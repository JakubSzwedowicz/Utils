//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once
#include <iosfwd>
#include <memory>

template <typename Config>
class IConfigParser {
   public:
    virtual ~IConfigParser() = default;

    virtual std::shared_ptr<Config> readConfig(std::istream& stream) const = 0;
    virtual int writeConfig(const Config& config, std::ostream& out) const = 0;
};