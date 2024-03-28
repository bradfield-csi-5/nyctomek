cmake_minimum_required(VERSION 3.20)

# Appease the DOWNLOAD_EXTRACT_TIMESTAMP warning.
cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(level2db_tests level2db_tests.cpp)

target_link_libraries(level2db_tests GTest::gtest_main level2db)

include(GoogleTest)
gtest_discover_tests(level2db_tests)

