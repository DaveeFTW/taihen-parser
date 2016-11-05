cmake_minimum_required(VERSION 3.1.0)

project(taihen-parser)

option(TEST "build and perform tests" OFF)
option(USE_INBUILT_CTYPE "use internal ctype over system" OFF)
option(USE_INBUILT_STRING "use internal string over system" OFF)

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
