add_executable(rapp collector.c eloop.c httpconnection.c httprequest.c httpserver.c main.c signalhandler.c tcpconnection.c tcpserver.c)

add_custom_target(
   http_parser
   COMMAND make library
   WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/contrib/http-parser"
)

target_link_libraries(rapp "${PROJECT_SOURCE_DIR}/contrib/http-parser/libhttp_parser.so.2.1")
include_directories("${PROJECT_SOURCE_DIR}/contrib/http-parser")
add_dependencies(rapp http_parser)

install (TARGETS rapp DESTINATION bin)
install (FILES "${PROJECT_SOURCE_DIR}/contrib/http-parser/libhttp_parser.so.2.1" DESTINATION lib)
