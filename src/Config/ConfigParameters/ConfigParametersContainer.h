//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "ConfigParameters/ConfigParameter.h"

namespace Utils::Config::ConfigParameters {

class ConfigParametersContainer {
   public:
    template <typename T>
    ConfigParameter<T> buildConfigParam(std::string name, std::string description, T defaultValue,
                                         std::function<bool(const T&)> validator = {}) {
        auto slot =
            std::make_unique<ConfigSlot<T>>(std::move(name), std::move(description), std::move(validator));
        auto* ptr = static_cast<ConfigSlot<T>*>(slot.get());
        m_slots.push_back(std::move(slot));
        m_defaultApplicators.push_back([ptr, defaultValue = std::move(defaultValue)] { ptr->setValue(defaultValue); });
        return ConfigParameter<T>{ptr};
    }

    void applyDefaults() {
        for (auto& fn : m_defaultApplicators) fn();
    }

    size_t size() const { return m_slots.size(); }

    IConfigSlotBase& at(size_t i) { return *m_slots[i]; }
    const IConfigSlotBase& at(size_t i) const { return *m_slots[i]; }

    bool equals(const ConfigParametersContainer& other) const {
        if (m_slots.size() != other.m_slots.size()) return false;
        for (size_t i = 0; i < m_slots.size(); ++i) {
            if (!m_slots[i]->equals(*other.m_slots[i])) return false;
        }
        return true;
    }

   private:
    std::vector<std::unique_ptr<IConfigSlotBase>> m_slots;
    std::vector<std::function<void()>> m_defaultApplicators;
};

}  // namespace Utils::Config::ConfigParameters
