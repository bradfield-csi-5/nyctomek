cmake_minimum_required(VERSION 3.29)

project(level2db)

add_library(level2db level2db_database.cpp level2db_iterator.cpp)

target_include_directories(level2db PUBLIC .)

add_executable(perftest perftest.cpp)
target_link_libraries(perftest level2db)

add_subdirectory(tests)

add_library(skiplist STATIC IMPORTED)
set_target_properties(skiplist PROPERTIES IMPORTED_LOCATION "/home/tomasz/Projects/nyctomek/storage-retrieval/skiplist/build/libskiplist.a")
target_link_libraries(level2db PUBLIC skiplist)
target_include_directories(level2db PUBLIC "/home/tomasz/Projects/nyctomek/storage-retrieval/skiplist")