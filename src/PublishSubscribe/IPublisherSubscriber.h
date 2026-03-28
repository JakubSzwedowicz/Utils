//
// Created by Jakub Szwedowicz on 10/6/25.
//

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_set>

namespace Utils::PublishSubscribe {

template <typename Message>
class PublishSubscribeManager;

template <typename Message>
class IPublisher {
   public:
    IPublisher();
    virtual ~IPublisher();

    virtual std::optional<Message> pull() const { return std::nullopt; }

   protected:
    virtual void publish(const Message& message);

   private:
    PublishSubscribeManager<Message>* m_manager = nullptr;
};

template <typename Message>
class StatefulPublisher : public IPublisher<Message> {
   public:
    StatefulPublisher() = default;
    ~StatefulPublisher() override = default;

    std::optional<Message> pull() const override;

   protected:
    void publish(const Message& message) override;

   private:
    mutable std::shared_mutex m_stateMutex;
    std::optional<Message> m_lastMessage;
};

template <typename Message>
class ISubscriber {
   public:
    ISubscriber();
    virtual ~ISubscriber();

    virtual void onUpdate(const Message& message) = 0;

    void pull();

   private:
    PublishSubscribeManager<Message>* m_manager = nullptr;
};

template <typename Message>
class PublishSubscribeManager {
   public:
    virtual ~PublishSubscribeManager() = default;

    void addPublisher(IPublisher<Message>* publisher) {
        if (publisher == nullptr) return;

        std::lock_guard lock(s_mutex);
        m_publishers.emplace(publisher);
    }

    void removePublisher(IPublisher<Message>* publisher) {
        if (publisher == nullptr) return;

        std::lock_guard lock(s_mutex);
        m_publishers.erase(publisher);
    }

    void addSubscriber(ISubscriber<Message>* subscriber) {
        if (subscriber == nullptr) return;

        std::lock_guard lock(s_mutex);
        m_subscribers.emplace(subscriber);
    }

    void removeSubscriber(ISubscriber<Message>* subscriber) {
        if (subscriber == nullptr) return;

        std::lock_guard lock(s_mutex);
        m_subscribers.erase(subscriber);
    }

    void publishMessage(const Message& message) {
        std::shared_lock lock(s_mutex);
        for (auto* subscriber : m_subscribers) {
            subscriber->onUpdate(message);
        }
    }

    std::optional<Message> pullMessage() const {
        std::shared_lock lock(s_mutex);
        for (auto* publisher : m_publishers) {
            if (auto msg = publisher->pull()) {
                return msg;
            }
        }
        return std::nullopt;
    }

    size_t getPublisherCount() const {
        std::shared_lock lock(s_mutex);
        return m_publishers.size();
    }

    size_t getSubscriberCount() const {
        std::shared_lock lock(s_mutex);
        return m_subscribers.size();
    }

    static PublishSubscribeManager<Message>* getManager() {
        if (!s_manager) {
            std::lock_guard lock(s_mutex);
            if (!s_manager) {
                s_manager = std::make_unique<PublishSubscribeManager<Message>>();
            }
        }
        return s_manager.get();
    }

    static void setManager(std::unique_ptr<PublishSubscribeManager<Message>> manager) {
        s_manager = std::move(manager);
    }

   private:
    static std::unique_ptr<PublishSubscribeManager<Message>> s_manager;
    static std::shared_mutex s_mutex;

    std::unordered_set<IPublisher<Message>*> m_publishers;
    std::unordered_set<ISubscriber<Message>*> m_subscribers;
};

// Static member definitions
template <typename Message>
std::unique_ptr<PublishSubscribeManager<Message>> PublishSubscribeManager<Message>::s_manager = nullptr;

template <typename Message>
std::shared_mutex PublishSubscribeManager<Message>::s_mutex;

// ======================= IMPLEMENTATION =======================

// IPublisher implementation
template <typename Message>
IPublisher<Message>::IPublisher() : m_manager(PublishSubscribeManager<Message>::getManager()) {
    m_manager->addPublisher(this);
}

template <typename Message>
IPublisher<Message>::~IPublisher() {
    m_manager->removePublisher(this);
}

template <typename Message>
void IPublisher<Message>::publish(const Message& message) {
    m_manager->publishMessage(message);
}

// StatefulPublisher implementation
template <typename Message>
std::optional<Message> StatefulPublisher<Message>::pull() const {
    std::shared_lock lock(m_stateMutex);
    return m_lastMessage;
}

template <typename Message>
void StatefulPublisher<Message>::publish(const Message& message) {
    {
        std::lock_guard lock(m_stateMutex);
        m_lastMessage = message;
    }
    IPublisher<Message>::publish(message);
}

// ISubscriber implementation
template <typename Message>
ISubscriber<Message>::ISubscriber() : m_manager(PublishSubscribeManager<Message>::getManager()) {
    m_manager->addSubscriber(this);
}

template <typename Message>
ISubscriber<Message>::~ISubscriber() {
    m_manager->removeSubscriber(this);
}

template <typename Message>
void ISubscriber<Message>::pull() {
    if (auto msg = m_manager->pullMessage()) {
        this->onUpdate(*msg);
    }
}

}  // namespace Utils::PublishSubscribe
