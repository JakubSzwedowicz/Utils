//
// Created by Jakub Szwedowicz on 10/7/25.
//

#pragma once

#include <memory>

#include "IConfigProvider.h"
#include "PublishSubscribe/IPublisherSubscriber.h"

namespace Utils::Config {

template <typename Config>
class ConfigPublisher : public IConfigProvider<Config>, public PublishSubscribe::IPublisher<std::shared_ptr<Config>> {
   public:
    ~ConfigPublisher() override = default;

    void setConfig(std::shared_ptr<Config> config) override {
        IConfigProvider<Config>::setConfig(config);
        this->publish(config);
    }
};

}  // namespace Utils::Config