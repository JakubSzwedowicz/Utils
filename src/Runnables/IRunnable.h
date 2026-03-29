//
// Created by Jakub Szwedowicz on 3/29/26.
//

#pragma once

namespace Utils::Runnables {

class IRunnable {
   public:
    virtual ~IRunnable() = default;
    virtual void run() = 0;
};

}  // namespace Utils::Runnables
