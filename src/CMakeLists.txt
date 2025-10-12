add_subdirectory(PublishSubscribe)
add_subdirectory(Logging)
add_subdirectory(Config)

add_library(Utils INTERFACE)
add_library(Utils::Utils ALIAS Utils)

target_compile_features(Utils INTERFACE cxx_std_23)

target_link_libraries(Utils
    INTERFACE
        Utils::Config
        Utils::Logging
        Utils::PublishSubscribe
)

target_include_directories(Utils
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS Config Logging ConfigParser PublishSubscribe
    EXPORT UtilsTargets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Utils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT UtilsTargets
    FILE UtilsTargets.cmake
    NAMESPACE Utils::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Utils
)

# Optional: Keep the App executable for development/testing
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_executable(App main.cpp)
    target_link_libraries(App PRIVATE Utils::Utils)
endif()
