# PublishSubscribe library
add_library(PublishSubscribe INTERFACE)
add_library(Utils::PublishSubscribe ALIAS PublishSubscribe)

target_sources(PublishSubscribe
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
        FILES
            IPublisherSubscriber.h
)
target_include_directories(PublishSubscribe
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)
