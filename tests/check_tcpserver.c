#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <eloop.h>
#include <tcpserver.h>

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define PORT_S_LEN STRLEN("65535")

#define HOST "localhost"
#define PORT 8000


struct ELoop *eloop = NULL;
struct TcpServer *tcp_server = NULL;
struct TcpConnection *tcp_connection = NULL;


static int
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

static void
accept_func(struct TcpConnection *connection,
            const void           *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  tcp_connection = connection;

  event_loop_stop(eloop);
}

void
setup(void)
{
  eloop = event_loop_new();
  tcp_server = tcp_server_new(eloop);
  tcp_connection = NULL;
}

void teardown(void)
{
  tcp_connection_destroy(tcp_connection);
  tcp_server_destroy(tcp_server);
  event_loop_destroy(eloop);
}

START_TEST(test_tcp_server_calls_callback_when_accepts_new_connections)
{
  int client_fd = -1;

  tcp_server_set_accept_callback(tcp_server, accept_func, eloop);
  tcp_server_start_listen(tcp_server, HOST, PORT);

  ck_assert_int_ge(connect_to(HOST, PORT), 0);

  event_loop_run(eloop);

  ck_assert_ptr_ne(tcp_connection, NULL);
}
END_TEST


static Suite *
tcp_server_suite(void)
{
  Suite *s = suite_create("rapp.core.tcpserver");
  TCase *tc = tcase_create("rapp.core.tcpserver");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_tcp_server_calls_callback_when_accepts_new_connections);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = tcp_server_suite ();
 SRunner *sr = srunner_create (s);

 srunner_run_all (sr, CK_NORMAL);
 number_failed = srunner_ntests_failed (sr);
 srunner_free (sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
