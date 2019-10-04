cmake_minimum_required(VERSION 3.10)
project(example)

# Disable building utf8streams tests
set(UTF8STREAMS_BUILD_TESTS OFF CACHE BOOL "")

# Add utf8streams as sub directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/utf8streams)

# Define our executable and link to utf8streams
add_executable(example
        main.cpp
        )

target_link_libraries(example utf8streams)
