cmake_minimum_required(VERSION 3.29)

project(level2db)

add_library(level2db level2db_database.cpp level2db_iterator.cpp)

target_include_directories(level2db PUBLIC .)

add_subdirectory(tests)

