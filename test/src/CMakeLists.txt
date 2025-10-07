add_executable(
    UtilsConfigTest
    testConfigProvider.cpp
    testConfigPublisher.cpp
    testJsonConfigParser.cpp
)

target_link_libraries(
    UtilsConfigTest
    PRIVATE
    Utils
    GTest::gtest_main
)

# Add the test to CTest
gtest_discover_tests(UtilsConfigTest)
