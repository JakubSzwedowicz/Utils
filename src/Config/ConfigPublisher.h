//
// Created by Jakub Szwedowicz on 10/7/25.
//

#pragma once

#include <memory>

#include "PublishSubscribe/IPublisherSubscriber.h"

namespace Utils::Config {

template <typename Config>
class ConfigPublisher : public PublishSubscribe::StatefulPublisher<std::shared_ptr<const Config>> {
   public:
    ~ConfigPublisher() override = default;

    virtual void setConfig(std::shared_ptr<const Config> config) {
        this->publish(config);
    }

    virtual std::shared_ptr<const Config> getConfig() const {
        return this->pull().value_or(nullptr);
    }
};

}  // namespace Utils::Config
