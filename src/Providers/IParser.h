//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

namespace Utils::Providers {

template <typename Source, typename Output>
class IParser {
   public:
    virtual ~IParser() = default;
    virtual Output parse(Source source) = 0;
};

}  // namespace Utils::Providers
