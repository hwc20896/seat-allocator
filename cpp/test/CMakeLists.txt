cmake_minimum_required(VERSION 3.31)
project(grid_shuffler_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(grid_shuffler_tests test.cpp)

enable_testing()

target_link_libraries(grid_shuffler_tests PRIVATE GTest::GTest GTest::Main grid_shuffler)

add_test(NAME grid_shuffler_tests COMMAND grid_shuffler_tests)