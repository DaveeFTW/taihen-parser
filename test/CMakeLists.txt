find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(${taihen-config_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(test-lexer test_lexer.cpp)
target_link_libraries(test-lexer taihenconfig)

add_executable(test-parser test_parser.cpp)
target_link_libraries(test-parser taihenconfig)
