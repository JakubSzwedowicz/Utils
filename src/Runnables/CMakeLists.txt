add_library(Runnables INTERFACE)
add_library(Utils::Runnables ALIAS Runnables)

target_sources(Runnables
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
        FILES
            IRunnable.h
)

target_include_directories(Runnables
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)
