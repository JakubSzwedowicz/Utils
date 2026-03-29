//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

#include <memory>
#include <optional>
#include <queue>

#include "IParser.h"
#include "IResourceProvider.h"
#include "ISourceProvider.h"

namespace Utils::Providers {

template <typename Resource, typename Source>
class QueuedResourceProvider : public IResourceProvider<Resource> {
   public:
    QueuedResourceProvider(std::unique_ptr<ISourceProvider<Source>> source,
                           std::unique_ptr<IParser<Source, Resource>> parser)
        : m_source(std::move(source)), m_parser(std::move(parser)) {}

    void run() override {
        m_source->run();
        while (auto raw = m_source->poll()) {
            m_queue.push(m_parser->parse(std::move(*raw)));
        }
    }

    std::optional<Resource> poll() override {
        if (m_queue.empty()) return std::nullopt;
        auto front = std::move(m_queue.front());
        m_queue.pop();
        return front;
    }

   private:
    std::unique_ptr<ISourceProvider<Source>> m_source;
    std::unique_ptr<IParser<Source, Resource>> m_parser;
    std::queue<Resource> m_queue;
};

}  // namespace Utils::Providers
