option(ENABLE_CONTAINER_STREAM "compile stream container")

if(ENABLE_CONTAINER_STREAM)
  set(CMAKE_SHARED_MODULE_PREFIX "")
  add_library(stream MODULE rapp_plugin.c)
  include_directories(${PROJECT_SOURCE_DIR})
endif(ENABLE_CONTAINER_STREAM)
