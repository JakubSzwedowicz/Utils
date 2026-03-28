#pragma once

#include "Logger.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

namespace Utils::Logging {

class LoggerSubscribed : public Logger, public PublishSubscribe::ISubscriber<std::shared_ptr<LoggerConfig>> {
   public:
    explicit LoggerSubscribed(std::string name) : Logger(std::move(name), nullptr) {}

    void onUpdate(const std::shared_ptr<LoggerConfig>& message) override { Logger::onUpdate(message); }
};

}  // namespace Utils::Logging
