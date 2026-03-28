//
// Created by Jakub Szwedowicz on 10/7/25.
//

#pragma once

#include <memory>
#include <mutex>

#include "PublishSubscribe/IPublisherSubscriber.h"

namespace Utils::Config {

template <typename Config>
class ConfigPublisher : public PublishSubscribe::IPublisher<std::shared_ptr<Config>> {
   public:
    ~ConfigPublisher() override = default;

    virtual void setConfig(std::shared_ptr<Config> config) {
        {
            std::lock_guard lock(m_mutex);
            m_config = config;
        }
        this->publish(config);
    }

    virtual std::shared_ptr<Config> getConfig() const {
        std::lock_guard lock(m_mutex);
        return m_config;
    }

   protected:
    mutable std::mutex m_mutex;
    std::shared_ptr<Config> m_config;
};

}  // namespace Utils::Config
