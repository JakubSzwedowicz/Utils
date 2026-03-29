//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "IParser.h"
#include "IResourceProvider.h"
#include "ISourceProvider.h"

namespace Utils::Providers {

template <typename Resource, typename Source>
class ResourceProvider : public IResourceProvider<Resource> {
   public:
    ResourceProvider(std::unique_ptr<ISourceProvider<Source>> source,
                     std::unique_ptr<IParser<Source, Resource>> parser)
        : m_source(std::move(source)), m_parser(std::move(parser)) {}

    void run() override {
        m_source->run();
        if (auto raw = m_source->poll()) {
            m_pending = m_parser->parse(std::move(*raw));
        }
    }

    std::optional<Resource> poll() override { return std::exchange(m_pending, std::nullopt); }

   private:
    std::unique_ptr<ISourceProvider<Source>> m_source;
    std::unique_ptr<IParser<Source, Resource>> m_parser;
    std::optional<Resource> m_pending;
};

}  // namespace Utils::Providers
