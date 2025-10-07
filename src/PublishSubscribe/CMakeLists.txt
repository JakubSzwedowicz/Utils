# PublishSubscribe library
add_library(PublishSubscribe INTERFACE)
add_library(Utils::PublishSubscribe ALIAS PublishSubscribe)

target_sources(PublishSubscribe
    INTERFACE
        FILE_SET HEADERS
        FILES
            IPublisherSubscriber.h
)
target_include_directories(PublishSubscribe
    INTERFACE
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
