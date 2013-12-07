set(CMAKE_SHARED_MODULE_PREFIX "")
add_library(hello MODULE rapp_plugin.c)
include_directories(${PROJECT_SOURCE_DIR})
