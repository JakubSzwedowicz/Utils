cmake_minimum_required(VERSION 3.10...3.31)

project(Utils
    VERSION 1.0.0
    DESCRIPTION "A utility library with ConfigReader and Logger components"
    LANGUAGES CXX)

add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    add_subdirectory(test)
endif()


# Export targets for find_package
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate the config file that includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UtilsConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/UtilsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Utils
)

# Generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/UtilsConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY AnyNewerVersion
)

# Install the configuration files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/UtilsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/UtilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Utils
)
