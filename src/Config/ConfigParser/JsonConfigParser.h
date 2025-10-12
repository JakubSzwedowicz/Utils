//
// Created by Jakub Szwedowicz on 10/2/25.
//
#pragma once

#include <iostream>
#include <string>

#include "IConfigParser.h"
#include "glaze/glaze.hpp"

namespace Utils {
namespace Config {

template <typename Config>
class JsonConfigParser : public IConfigParser<Config> {
   public:
    JsonConfigParser() { static_assert(glz::detail::reflectable<Config>); }

    std::shared_ptr<Config> readConfig(std::istream &jsonStream) const {
        std::string json((std::istreambuf_iterator<char>(jsonStream)),
                        std::istreambuf_iterator<char>());

        Config config;
        auto ec = glz::read_json(config, json);
        if (ec) {
            // Fix: Correct error message for reading (not writing)
            std::cerr << "Error reading JSON config: " << static_cast<uint32_t>(ec);

            if (ec == glz::error_code::unknown_key) {
                std::cerr << " (unknown_key - JSON contains fields not in struct)";
            }

            std::cerr << std::endl;
            return nullptr;
        }
        return std::make_shared<Config>(config);
    }

    int writeConfig(const Config &config, std::ostream &out) const {
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

}  // namespace Config
}  // namespace Utils