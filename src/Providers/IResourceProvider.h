//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

#include <optional>

#include "Runnables/IRunnable.h"

namespace Utils::Providers {

template <typename Resource>
class IResourceProvider : public Runnables::IRunnable {
   public:
    virtual std::optional<Resource> poll() = 0;
};

}  // namespace Utils::Providers
