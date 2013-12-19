
set(HTTP_PARSER_DIR ${PROJECT_SOURCE_DIR}/contrib/http-parser)
set(HTTP_PARSER_SOURCES ${HTTP_PARSER_DIR}/http_parser.c)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.c.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.c" @ONLY)

set(RAPP_CORE_SOURCES
    collector.c
    container.c
    eloop.c
    httpconnection.c
    httpresponsewriter.c
    httprequest.c
    httpserver.c
    signalhandler.c
    tcpconnection.c
    tcpserver.c
    logger.c
    version.c
    ${HTTP_PARSER_SOURCES})

add_library(rapp_core STATIC ${RAPP_CORE_SOURCES})
add_executable(rapp main.c)

set_target_properties(rapp PROPERTIES COMPILE_FLAGS "-export-dynamic")

target_link_libraries(rapp rapp_core dl)

include_directories(${HTTP_PARSER_DIR})
include_directories(${PROJECT_SOURCE_DIR})

install(TARGETS rapp DESTINATION bin)
