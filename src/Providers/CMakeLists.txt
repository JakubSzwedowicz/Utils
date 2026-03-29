add_library(Providers STATIC)
add_library(Utils::Providers ALIAS Providers)

target_sources(Providers
    PRIVATE
        FileSourceProvider.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
        FILES
            IParser.h
            ISourceProvider.h
            IResourceProvider.h
            ResourceProvider.h
            QueuedResourceProvider.h
            FileSourceProvider.h
)

target_compile_features(Providers PRIVATE cxx_std_23)

target_link_libraries(Providers
    PUBLIC  Utils::Runnables
    PRIVATE Utils::Logging
)

target_include_directories(Providers
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)
