#include <stdlib.h>
#include <check.h>

#include <test_tcp_client.h>
#include <eloop.h>
#include <tcpserver.h>


#define HOST "localhost"
#define PORT 8000


struct ELoop *eloop = NULL;
struct TcpServer *tcp_server = NULL;
struct TcpConnection *tcp_connection = NULL;


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
