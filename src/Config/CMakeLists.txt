add_library(Config INTERFACE)
add_library(Utils::Config ALIAS Config)

target_sources(Config
    PRIVATE
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
        FILES
            ConfigPublisher.h
            ConfigManager.h
            ConfigParameters/ConfigParameter.h
            ConfigParameters/ConfigParametersContainer.h
            ConfigProviders/IConfigProvider.h
            ConfigProviders/DefaultConfigProvider.h
            ConfigProviders/CLIConfigProvider.h
            ConfigProviders/JsonConfigProvider.h
)

target_include_directories(Config
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)

add_subdirectory(ConfigParser)
target_link_libraries(Config
    INTERFACE
        Utils::ConfigParser
        Utils::Providers
        Utils::Logging
        Utils::Runnables
)
