include(CheckIncludeFile)

if (MSVC)
    add_definitions(-Dinline=__inline)
endif()

check_include_file(ctype.h HAVE_CTYPE)
check_include_file(string.h HAVE_STRING)

if ((NOT ${HAVE_CTYPE}) OR ${USE_INBUILT_CTYPE})
    add_definitions(-DNO_CTYPE)
endif()

if ((NOT ${HAVE_STRING}) OR ${USE_INBUILT_STRING})
    add_definitions(-DNO_STRING)
endif()

add_definitions(-Os -ffunction-sections -fdata-sections)

add_library(taihenparser lexer.c parser.c)

if (${INSTALL_ENABLED})
install(TARGETS taihenparser
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif()
