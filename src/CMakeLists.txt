
set(HTTP_PARSER_DIR ${PROJECT_SOURCE_DIR}/contrib/http-parser)
set(HTTP_PARSER_SOURCES ${HTTP_PARSER_DIR}/http_parser.c)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.c.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.c" @ONLY)

set(RAPP_CORE_SOURCES
    collector.c
    config/api.c
    config/commandline.c
    config/common.c
    config/env.c
    config/yaml.c
    container.c
    eloop.c
    httpconnection.c
    httpresponse.c
    httprequest.c
    httprequestqueue.c
    httprouter.c
    httpserver.c
    logger.c
    signalhandler.c
    tcpconnection.c
    tcpserver.c
    version.c
    ${HTTP_PARSER_SOURCES})

add_library(rapp_core STATIC ${RAPP_CORE_SOURCES})
add_executable(rapp main.c memory.c)
# memory.c MUST be outside of rapp_core and must be executable-specific.

set_target_properties(rapp_core PROPERTIES COMPILE_FLAGS "-Wall -export-dynamic")

target_link_libraries(rapp rapp_core dl)

include_directories(${HTTP_PARSER_DIR})
include_directories(${PROJECT_SOURCE_DIR})
target_link_libraries(rapp ${LIBYAML_LIBRARIES})

install(TARGETS rapp DESTINATION bin)
