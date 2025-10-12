# Logging library
add_library(Logging STATIC)

add_library(Utils::Logging ALIAS Logging)

target_sources(Logging
        PRIVATE
        Logger.cpp
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
        FILES
        Logger.h
        LoggerConfig.h
        LogLevel.h
        LoggerMacros.h
)

find_package(spdlog REQUIRED)
target_link_libraries(Logging PUBLIC spdlog::spdlog)
target_compile_features(Logging PUBLIC cxx_std_20)
target_include_directories(Logging
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)
