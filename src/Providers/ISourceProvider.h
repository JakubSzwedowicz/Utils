//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

#include <optional>

#include "Runnables/IRunnable.h"

namespace Utils::Providers {

template <typename Source>
class ISourceProvider : public Runnables::IRunnable {
   public:
    virtual std::optional<Source> poll() = 0;
};

}  // namespace Utils::Providers
