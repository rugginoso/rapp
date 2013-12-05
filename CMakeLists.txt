cmake_minimum_required(VERSION 2.8)
project(rapp)

include(CheckFunctionExists)
include (CheckCSourceCompiles)

CHECK_FUNCTION_EXISTS(epoll_create HAVE_EPOLL_CREATE)
if (NOT HAVE_EPOLL_CREATE)
  message(FATAL_ERROR "epoll_create not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT HAVE_EPOLL_CREATE)

CHECK_FUNCTION_EXISTS(epoll_ctl HAVE_EPOLL_CTL)
if (NOT HAVE_EPOLL_CTL)
  message(FATAL_ERROR "epoll_ctl not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT HAVE_EPOLL_CTL)

CHECK_FUNCTION_EXISTS(eventfd HAVE_EVENTFD)
if (NOT HAVE_EVENTFD)
  message(FATAL_ERROR "eventfd not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT HAVE_EVENTFD)

CHECK_FUNCTION_EXISTS(signalfd HAVE_SIGNALFD)
if (NOT HAVE_SIGNALFD)
  message(FATAL_ERROR "signalfd not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT HAVE_SIGNALFD)

check_c_source_compiles(
  "
  #include <sys/types.h>
  #include <sys/socket.h>
  int main(void)
  {
    int on = 1;
    setsockopt(0, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int));
    return 0;
  }
  " HAVE_SO_REUSEPORT)
  if (NOT HAVE_SO_REUSEPORT)
  message(FATAL_ERROR "signalfd not found. WARNING! rapp runs on modern linuxes only!")
endif (NOT HAVE_SO_REUSEPORT)

add_subdirectory(src)
add_subdirectory(containers/hello)
