#include <stdio.h>
#include <alloca.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "test_tcp_client.h"


#define STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define PORT_S_LEN STRLEN("65535")


int
connect_to(const char *host, uint16_t port)
{
  struct addrinfo *addrinfos, hints = {0, };
  char *port_s = NULL;
  int addrinfo_ret = 0;
  int fd = -1;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  port_s = alloca(PORT_S_LEN);
  snprintf(port_s, PORT_S_LEN, "%d", port);

  if ((addrinfo_ret = getaddrinfo(host, port_s, &hints, &addrinfos)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo_ret));
    return -1;
  }

  if ((fd = socket(addrinfos->ai_family, addrinfos->ai_socktype, 0)) < 0) {
    perror("socket");
    freeaddrinfo(addrinfos);
    return -1;
  }

  if (connect(fd, addrinfos->ai_addr, addrinfos->ai_addrlen) < 0) {
    perror("connect");
    freeaddrinfo(addrinfos);
    return -1;
  }
  freeaddrinfo(addrinfos);

  return fd;
}
