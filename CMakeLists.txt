cmake_minimum_required(VERSION 3.10)
project(certifiable-harness VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Production flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffp-contract=off -fno-fast-math")

# Library
add_library(ch_lib STATIC
    src/harness.c
    src/golden.c
    src/report.c
    src/stages.c
    src/hash.c
    src/sha256.c
    src/sha256.c
)

target_include_directories(ch_lib PUBLIC include)

# Main executable
add_executable(certifiable-harness src/main.c)
target_link_libraries(certifiable-harness ch_lib)

# Tests
enable_testing()

add_executable(test_harness tests/unit/test_harness.c)
target_link_libraries(test_harness ch_lib)
add_test(NAME test_harness COMMAND test_harness)

add_executable(test_golden tests/unit/test_golden.c)
target_link_libraries(test_golden ch_lib)
add_test(NAME test_golden COMMAND test_golden)

add_executable(test_stages tests/unit/test_stages.c)
target_link_libraries(test_stages ch_lib)
add_test(NAME test_stages COMMAND test_stages)

add_executable(test_report tests/unit/test_report.c)
target_link_libraries(test_report ch_lib)
add_test(NAME test_report COMMAND test_report)
