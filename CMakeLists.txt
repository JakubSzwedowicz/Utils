cmake_minimum_required(VERSION 3.10...3.31)

project(HelloWorld LANGUAGES CXX)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)