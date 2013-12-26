project(rapp)
cmake_minimum_required(VERSION 2.8)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_get_exact_tag(GIT_TAG)

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

if(CMAKE_COMPILER_IS_GNUCC)
  option(ENABLE_TESTS_COVERAGE "build test coverage report")
  if (ENABLE_TESTS_COVERAGE)
    message(STATUS "Enabling the code coverage support")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    include(TestCoverage)
  endif(ENABLE_TESTS_COVERAGE)
endif(CMAKE_COMPILER_IS_GNUCC)

add_subdirectory(containers/hello)
add_subdirectory(containers/python)
add_subdirectory(src)
add_subdirectory(tests)
