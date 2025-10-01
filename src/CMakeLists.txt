find_package(fmt CONFIG REQUIRED)

add_executable(App main.cpp)

target_link_libraries(App PRIVATE fmt::fmt Calc)

add_subdirectory(Calc)