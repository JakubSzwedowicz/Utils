#include <iostream>

#include "Logging/LoggerMacros.h"

int main() {
    // Defines the logger instance for the macros in this scope
    auto& m_logger = Utils::Logging::Logger::getInstance();

    std::cout << "Hello, World!" << std::endl;

    LOG_I("This is an info log. {}", 123);
    LOG_D("Some debug {} print", "YOLO");
    LOG_W("Some warn {} print", "YOLOLOLO");
    LOG_E("Some error {} print", "DELULU");
    LOG_C("Some critical {} print", "CRITICALLLL");
}