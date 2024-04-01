cmake_minimum_required(VERSION 3.29)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(skiplist_tests skiplist_tests.cpp)

target_link_libraries(skiplist_tests GTest::gtest_main skiplist)

include(GoogleTest)
gtest_discover_tests(skiplist_tests)
