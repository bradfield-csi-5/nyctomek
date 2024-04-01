cmake_minimum_required(VERSION 3.29)

project(skiplist)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(skiplist skiplist.cpp)

target_include_directories(skiplist PUBLIC ".")

add_subdirectory(tests)
