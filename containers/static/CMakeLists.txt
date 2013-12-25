option(ENABLE_CONTAINER_STATIC "compile static container")

if(ENABLE_CONTAINER_STATIC)
  set(CMAKE_SHARED_MODULE_PREFIX "")
  add_library(static MODULE rapp_plugin.c errors.c path.c mimetypes.c)
  include_directories(${PROJECT_SOURCE_DIR})
endif(ENABLE_CONTAINER_STATIC)
