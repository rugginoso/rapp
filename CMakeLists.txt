project(rapp)
cmake_minimum_required(VERSION 2.8)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCSourceCompiles)

CHECK_FUNCTION_EXISTS(epoll_create EPOLL_CREATE_FOUND)
if (NOT EPOLL_CREATE_FOUND)
  message(FATAL_ERROR "epoll_create not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT EPOLL_CREATE_FOUND)

CHECK_FUNCTION_EXISTS(epoll_ctl EPOLL_CTL_FOUND)
if (NOT EPOLL_CTL_FOUND)
  message(FATAL_ERROR "epoll_ctl not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT EPOLL_CTL_FOUND)

CHECK_FUNCTION_EXISTS(eventfd EVENTFD_FOUND)
if (NOT EVENTFD_FOUND)
  message(FATAL_ERROR "eventfd not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT EVENTFD_FOUND)

CHECK_FUNCTION_EXISTS(signalfd SIGNALFD_FOUND)
if (NOT SIGNALFD_FOUND)
  message(FATAL_ERROR "signalfd not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT SIGNALFD_FOUND)

CHECK_SYMBOL_EXISTS(SO_REUSEPORT sys/socket.h SO_REUSEPORT_FOUND)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_SOURCE_DIR}/config.h)

find_package(LIBYAML)
#if (NOT LIBYAML_FOUND)
#  message(FATAL_ERROR "please install libyaml from your package manager or from http://pyyaml.org")
#endif (NOT LIBYAML_FOUND)

option(ENABLE_TESTS "compile testsuite")

if(ENABLE_TESTS)
  enable_testing()
endif(ENABLE_TESTS)

add_subdirectory(containers/hello)
find_package(LibYAML REQUIRED)
include_directories(${LIBYAML_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(tests)
