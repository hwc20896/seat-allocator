cmake_minimum_required(VERSION 3.31)
project(grid_shuffler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v3.0.1
)

FetchContent_MakeAvailable(pybind11)

find_package(Python REQUIRED COMPONENTS Development.Module)

pybind11_add_module(grid_shuffler grid_shuffler_alg.cpp)