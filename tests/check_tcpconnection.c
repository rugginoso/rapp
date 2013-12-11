#include <stdlib.h>
#include <check.h>
#include <sys/socket.h>

#include <test_tcp_client.h>
#include <eloop.h>
#include <tcpconnection.h>

#include <test_tcp_client.h>
#include <test_tcp_server.h>

#define HOST "localhost"
#define PORT 8000

#define MESSAGE "Hello world!"
#define STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define MESSAGE_LEN STRLEN(MESSAGE)

struct ELoop *eloop = NULL;
struct TcpConnection *tcp_connection = NULL;
int server_fd = -1;
int client_fd = -1;
char buf[MESSAGE_LEN];

void
setup(void)
{
  eloop = event_loop_new();
  tcp_connection = NULL;

  server_fd = listen_to(HOST, PORT);
  client_fd = connect_to(HOST, PORT);

  tcp_connection = tcp_connection_with_fd(accept(server_fd, NULL, NULL), eloop);
}

void teardown(void)
{
  close(client_fd);
  close(server_fd);
  tcp_connection_destroy(tcp_connection);
  event_loop_destroy(eloop);
}

static void
on_read(struct TcpConnection *c, const void *data)
{
  tcp_connection_read_data(c, buf, MESSAGE_LEN);

  event_loop_stop(eloop);
}

static void
on_write(struct TcpConnection *c, const void *data)
{
  tcp_connection_write_data(c, MESSAGE, MESSAGE_LEN);

  event_loop_stop(eloop);
}

static void
on_close(struct TcpConnection *c, const void *data)
{
  event_loop_stop(eloop);
}


START_TEST(test_tcp_connection_calls_read_callback_when_there_are_incoming_data)
{
  tcp_connection_set_callbacks(tcp_connection, on_read, NULL, NULL, eloop);

  write(client_fd, MESSAGE, MESSAGE_LEN);

  event_loop_run(eloop);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST

START_TEST(test_tcp_connection_calls_write_callback_when_can_read)
{
  tcp_connection_set_callbacks(tcp_connection, NULL, on_write, NULL, eloop);

  event_loop_run(eloop);

  read(client_fd, buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST

START_TEST(test_tcp_connection_calls_close_callback_when_the_peer_disconnects)
{
  tcp_connection_set_callbacks(tcp_connection, NULL, NULL, on_close, eloop);

  close(client_fd);

  event_loop_run(eloop);
}
END_TEST


static Suite *
tcp_connection_suite(void)
{
  Suite *s = suite_create("rapp.core.tcpconnection");
  TCase *tc = tcase_create("rapp.core.tcpconnection");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_tcp_connection_calls_read_callback_when_there_are_incoming_data);
  tcase_add_test(tc, test_tcp_connection_calls_write_callback_when_can_read);
  tcase_add_test(tc, test_tcp_connection_calls_close_callback_when_the_peer_disconnects);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = tcp_connection_suite ();
 SRunner *sr = srunner_create (s);

 srunner_run_all (sr, CK_NORMAL);
 number_failed = srunner_ntests_failed (sr);
 srunner_free (sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
