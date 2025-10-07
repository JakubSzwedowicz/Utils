add_library(Config INTERFACE)
add_library(Utils::Config ALIAS Config)

target_sources(Config
    PRIVATE
    PUBLIC
        FILE_SET HEADERS
        FILES
            IConfigProvider.h
            ConfigManagers.h
)

target_include_directories(Config
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(ConfigParser)
target_link_libraries(Config
    INTERFACE
        Utils::ConfigParser
)