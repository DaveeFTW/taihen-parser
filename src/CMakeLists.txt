if (MSVC)
    add_definitions(-Dinline=__inline)
endif()

add_library(taihenconfig lexer.c parser.c)

install(TARGETS taihenconfig
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
