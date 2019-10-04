cmake_minimum_required(VERSION 3.10)
project(utf8streams)

option(UTF8STREAMS_BUILD_TESTS "Build utf8streams tests" ON)

add_library(utf8streams
        Include/utf8streams.hpp
        Source/utf8streams.cpp
        )

target_include_directories(utf8streams PUBLIC Include)

target_compile_features(utf8streams PUBLIC cxx_std_11)

target_compile_options(utf8streams PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -pedantic -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX>
        )

if (${UTF8STREAMS_BUILD_TESTS})
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)

    enable_testing()

    add_executable(utf8streamstests
            Tests/utf8tests.cpp
            )

    target_include_directories(utf8streamstests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(utf8streamstests ${GTEST_BOTH_LIBRARIES} Threads::Threads utf8streams)

    add_test(NAME utf8streamstests COMMAND utf8streamstests)
endif ()
