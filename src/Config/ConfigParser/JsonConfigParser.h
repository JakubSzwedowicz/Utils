//
// Created by Jakub Szwedowicz on 10/2/25.
//
#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "Providers/IParser.h"
#include "glaze/glaze.hpp"

namespace Utils::Config::ConfigParser {

template <typename Config>
class JsonConfigParser : public Utils::Providers::IParser<std::string, std::shared_ptr<Config>> {
   public:
    JsonConfigParser() {
        static_assert(glz::reflectable<Config> || glz::glaze_object_t<Config>,
                      "Config must be glaze-reflectable or have a glz::meta specialization");
    }

    std::shared_ptr<Config> parse(std::string json) override {
        auto config = std::make_shared<Config>();
        auto ec = glz::read_json(*config, json);
        if (ec) {
            std::cerr << "JsonConfigParser: error reading JSON: " << static_cast<uint32_t>(ec);
            if (ec == glz::error_code::unknown_key)
                std::cerr << " (unknown_key — JSON contains fields not in struct)";
            std::cerr << '\n';
            return nullptr;
        }
        return config;
    }

    std::shared_ptr<Config> readConfig(std::istream& stream) {
        std::string json((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>{});
        return parse(std::move(json));
    }

    int writeConfig(const Config& config, std::ostream& out) {
        std::string json;
        auto ec = glz::write_json(config, json);
        if (ec) {
            std::cerr << "JsonConfigParser: error writing JSON: " << ec << '\n';
        } else {
            out << json;
        }
        return ec;
    }
};

}  // namespace Utils::Config::ConfigParser
