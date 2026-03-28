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
        LoggerSubscribed.h
)

find_package(spdlog REQUIRED)
target_link_libraries(Logging PRIVATE spdlog::spdlog)
target_compile_features(Logging PRIVATE cxx_std_23)
target_include_directories(Logging
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)
