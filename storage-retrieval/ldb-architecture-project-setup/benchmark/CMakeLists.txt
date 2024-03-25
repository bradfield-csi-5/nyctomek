cmake_minimum_required(VERSION 3.29)

project(leveldb_benchmark)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(leveldb_benchmark leveldb_benchmark.cpp random_data_generator.cpp)

add_library(leveldb STATIC IMPORTED)
set_target_properties(leveldb PROPERTIES IMPORTED_LOCATION "/home/tomasz/Projects/leveldb/build/libleveldb.a")
target_link_libraries(leveldb_benchmark PUBLIC leveldb)

target_include_directories(leveldb_benchmark PUBLIC "." "/home/tomasz/Projects/leveldb/include")
