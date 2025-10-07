# Logger library
add_library(Logger STATIC)

add_library(Utils::Logger ALIAS Logger)

target_compile_features(Logger PUBLIC cxx_std_17)

target_sources(Logger
    PRIVATE
        Logger.cpp
    PUBLIC
        FILE_SET HEADERS
        FILES
            Logger.h
)

target_include_directories(Logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
