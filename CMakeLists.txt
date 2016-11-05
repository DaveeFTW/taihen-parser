cmake_minimum_required(VERSION 3.1.0)

project(taihen-parser)

option(TEST "build and perform tests" OFF)

include_directories(include)

add_subdirectory(src)

if (${TEST})
    add_subdirectory(test)

    enable_testing()
    add_test(NAME LexerTest COMMAND test-lexer)
endif()

install(DIRECTORY include/taihen/
    DESTINATION include/taihen
    FILES_MATCHING PATTERN "*.h")
