//
// Created by Jakub Szwedowicz on 10/2/25.
//
#pragma once

#include <iostream>
#include <string>

#include "IConfigParser.h"
#include "glaze/glaze.hpp"

namespace Utils::Config::ConfigParser {

template <typename Config>
class JsonConfigParser : public IConfigParser<Config, std::istream&> {
   public:
    JsonConfigParser() { static_assert(glz::reflectable<Config> || glz::glaze_object_t<Config>); }

    std::shared_ptr<Config> readConfig(std::istream& jsonStream) const override {
        std::string json((std::istreambuf_iterator<char>(jsonStream)), std::istreambuf_iterator<char>());

        auto config = std::make_shared<Config>();
        auto ec = glz::read_json(*config, json);
        if (ec) {
            std::cerr << "Error reading JSON config: " << static_cast<uint32_t>(ec);

            if (ec == glz::error_code::unknown_key) {
                std::cerr << " (unknown_key - JSON contains fields not in struct)";
            }

            std::cerr << std::endl;
            return nullptr;
        }
        return config;
    }

    int writeConfig(const Config& config, std::ostream& out) const {
        std::string json;
        auto ec = glz::write_json(config, json);
        if (ec) {
            std::cerr << "Error writing to JSON config: " << ec << std::endl;
        } else {
            out << json;
        }
        return ec;
    }
};

}  // namespace Utils::Config::ConfigParser
