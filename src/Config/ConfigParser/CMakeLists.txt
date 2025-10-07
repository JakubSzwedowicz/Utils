add_library(ConfigParser STATIC)
add_library(Utils::ConfigParser ALIAS ConfigParser)

target_sources(ConfigParser
        PRIVATE
            ConfigParser.cpp
        PUBLIC
        FILE_SET HEADERS
        FILES
        IConfigParser.h
        JsonConfigParser.h
)

include(dependencies.cmake)
target_link_libraries(ConfigParser PUBLIC glaze::glaze)
target_compile_features(ConfigParser PUBLIC cxx_std_23)
