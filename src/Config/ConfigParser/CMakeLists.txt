add_library(ConfigParser STATIC)
add_library(Utils::ConfigParser ALIAS ConfigParser)

target_sources(ConfigParser
        PRIVATE
            ConfigParser.cpp
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../..
        FILES
        IConfigParser.h
        JsonConfigParser.h
)

find_package(glaze REQUIRED)
target_link_libraries(ConfigParser PUBLIC glaze::glaze)
target_compile_features(ConfigParser PUBLIC cxx_std_23)

target_include_directories(ConfigParser
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Utils>
)