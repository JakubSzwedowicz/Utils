//
// Created by Jakub Szwedowicz on 10/2/25.
//

#pragma once

#include <memory>
#include <mutex>

namespace Utils {
namespace Config {

template <typename Config>
class IConfigProvider {
public:
  virtual ~IConfigProvider() = default;

  virtual void setConfig(std::shared_ptr<Config> config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;
  }

  virtual std::shared_ptr<Config> getConfig() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config;
  }

protected:
  mutable std::mutex m_mutex;
  std::shared_ptr<Config> m_config;
};

} // namespace Config
} // namespace Utils