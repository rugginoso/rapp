
set(HTTP_PARSER_DIR ${PROJECT_SOURCE_DIR}/contrib/http-parser)
set(HTTP_PARSER_SOURCES ${HTTP_PARSER_DIR}/http_parser.c)

add_executable(rapp
				collector.c
				container.c
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
				${HTTP_PARSER_SOURCES})

include_directories(${HTTP_PARSER_DIR})

install (TARGETS rapp DESTINATION bin)
