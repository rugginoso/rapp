
set(HTTP_PARSER_DIR ${PROJECT_SOURCE_DIR}/contrib/http-parser)
set(HTTP_PARSER_SOURCES ${HTTP_PARSER_DIR}/http_parser.c)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.c.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.c" @ONLY)

add_executable(rapp
				collector.c
				container.c
                config.c
                config_yaml_parser.c
                config_private.c
				eloop.c
				httpconnection.c
				httpresponsewriter.c
				httprequest.c
				httpserver.c
				main.c
				signalhandler.c
				tcpconnection.c
				tcpserver.c
				logger.c
                version.c
				${HTTP_PARSER_SOURCES})

set_target_properties(rapp
				PROPERTIES COMPILE_FLAGS "-export-dynamic")

target_link_libraries(rapp dl)

include_directories(${HTTP_PARSER_DIR})
include_directories(${PROJECT_SOURCE_DIR})
target_link_libraries(rapp ${LIBYAML_LIBRARIES})

install (TARGETS rapp DESTINATION bin)
