//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>

#include "Providers/IParser.h"

namespace Utils::Config::ConfigParser {

template <typename Config, typename Source>
using IConfigParser = Utils::Providers::IParser<Source, std::shared_ptr<Config>>;

}  // namespace Utils::Config::ConfigParser
