
cmake_minimum_required(VERSION 3.25)
project(simple_http_parser)

set(CMAKE_CXX_STANDARD 17)
set(GTEST_DIR "/home/tomikaa/github/googletest")

add_executable(simple_http_parser main.cpp)

target_include_directories(simple_http_parser
    PRIVATE
        ${GTEST_DIR}/googletest/
        ${GTEST_DIR}/googlemock
        ${GTEST_DIR}/googletest/include
        ${GTEST_DIR}/googlemock/include
)

target_sources(simple_http_parser
    PRIVATE
        ${GTEST_DIR}/googletest/src/gtest-all.cc
        ${GTEST_DIR}/googlemock/src/gmock-all.cc
)