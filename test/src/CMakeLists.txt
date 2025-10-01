add_executable(
  ExampleTest
  exampleTest.cpp
)
target_link_libraries(
  ExampleTest
  GTest::gtest_main
)

gtest_discover_tests(ExampleTest)