//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>

namespace Utils::Config {

class IConfigSlotBase {
   public:
    virtual ~IConfigSlotBase() = default;

    virtual const std::string& name() const = 0;
    virtual const std::string& description() const = 0;
    virtual bool hasValue() const = 0;
    virtual bool validate() const = 0;
    virtual void copyValueFrom(const IConfigSlotBase& other) = 0;
    virtual bool equals(const IConfigSlotBase& other) const = 0;
    virtual void reset() = 0;
    virtual std::string valueToString() const = 0;
};

template <typename T>
class ConfigSlot : public IConfigSlotBase {
   public:
    ConfigSlot(std::string name, std::string description, std::function<bool(const T&)> validator = {})
        : m_name(std::move(name)), m_description(std::move(description)), m_validator(std::move(validator)) {}

    const std::string& name() const override { return m_name; }
    const std::string& description() const override { return m_description; }
    bool hasValue() const override { return m_value.has_value(); }

    bool validate() const override {
        if (!m_value.has_value()) return true;
        return !m_validator || m_validator(*m_value);
    }

    void copyValueFrom(const IConfigSlotBase& other) override {
        m_value = static_cast<const ConfigSlot<T>&>(other).m_value;
    }

    bool equals(const IConfigSlotBase& other) const override {
        return m_value == static_cast<const ConfigSlot<T>&>(other).m_value;
    }

    void reset() override { m_value.reset(); }

    std::string valueToString() const override {
        if (!m_value.has_value()) return "(unset)";
        if constexpr (std::is_same_v<T, bool>) {
            return *m_value ? "true" : "false";
        } else if constexpr (requires(std::ostream& os, const T& v) { os << v; }) {
            std::ostringstream oss;
            oss << *m_value;
            return oss.str();
        } else {
            return "<non-stringifiable>";
        }
    }

    void setValue(T value) { m_value = std::move(value); }
    const std::optional<T>& get() const { return m_value; }

   private:
    std::string m_name;
    std::string m_description;
    std::function<bool(const T&)> m_validator;
    std::optional<T> m_value;
};

template <typename T>
class ConfigParameter {
   public:
    explicit ConfigParameter(ConfigSlot<T>* slot) : m_slot(slot) {}

    const T& get() const {
        assert(m_slot->hasValue() &&
               "ConfigParameter has no value — ensure a default was provided and ConfigManager has resolved");
        return *m_slot->get();
    }

    void set(T value) { m_slot->setValue(std::move(value)); }
    void reset() { m_slot->reset(); }
    bool validate() const { return m_slot->validate(); }
    bool hasValue() const { return m_slot->hasValue(); }
    const std::string& name() const { return m_slot->name(); }
    const std::string& description() const { return m_slot->description(); }

   private:
    ConfigSlot<T>* m_slot;
};

}  // namespace Utils::Config
