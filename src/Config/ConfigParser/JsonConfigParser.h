//
// Created by Jakub Szwedowicz on 10/2/25.
//
#pragma once

#include <memory>
#include <string>

#include "Logging/Logger.h"
#include "Logging/LoggerMacros.h"
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

        if (ec == glz::error_code::unknown_key) {
            LOG_W("JsonConfigParser: JSON contains unrecognized field(s): {}", glz::format_error(ec, json));
            config = std::make_shared<Config>();
            (void)glz::read<glz::opts{.error_on_unknown_keys = false}>(*config, json);
            return config;
        }

        if (ec) {
            LOG_E("JsonConfigParser: failed to parse JSON: {}", glz::format_error(ec, json));
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
            LOG_E("JsonConfigParser: error writing JSON: {}", glz::format_error(ec, json));
        } else {
            out << json;
        }
        return ec;
    }

   private:
    mutable Utils::Logging::Logger m_logger{"JsonConfigParser"};
};

}  // namespace Utils::Config::ConfigParser
