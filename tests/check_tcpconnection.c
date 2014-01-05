/*
 * check_tcpconnection.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <check.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <logger.h>
#include <eloop.h>
#include <tcpconnection.h>

#include "test_memstubs.h"
#include "test_utils.h"

#define HOST "localhost"
#define PORT 8000

#define MESSAGE "Hello world!"
#define MESSAGE_LEN STRLEN(MESSAGE)

struct ELoop *eloop = NULL;
struct Logger *logger = NULL;
struct TcpConnection *tcp_connection = NULL;
int server_fd = -1;
int client_fd = -1;
char buf[MESSAGE_LEN];

void
setup(void)
{
  logger = logger_new_null();
  eloop = event_loop_new(logger);
  tcp_connection = NULL;

  server_fd = listen_to(HOST, PORT);
  client_fd = connect_to(HOST, PORT);

  tcp_connection = tcp_connection_with_fd(accept(server_fd, NULL, NULL), logger, eloop);

  memset(buf, 0, MESSAGE_LEN);
}

void teardown(void)
{
  close(client_fd);
  close(server_fd);
  tcp_connection_destroy(tcp_connection);
  event_loop_destroy(eloop);
  logger_destroy(logger);
}

static void
on_read(struct TcpConnection *c, const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  tcp_connection_read_data(c, buf, MESSAGE_LEN);

  event_loop_stop(eloop);
}

static void
on_write(struct TcpConnection *c, const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  tcp_connection_write_data(c, MESSAGE, MESSAGE_LEN);

  event_loop_stop(eloop);
}

static void
on_close(struct TcpConnection *c, const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

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

START_TEST(test_tcp_connection_sendfile)
{
  int file_fd = open("test_file.txt", O_WRONLY | O_CREAT, 0640);

  write(file_fd, MESSAGE, MESSAGE_LEN);
  close(file_fd);


  file_fd = open("test_file.txt", O_RDONLY);

  tcp_connection_sendfile(tcp_connection, file_fd, MESSAGE_LEN);

  close(file_fd);
  unlink("test_file.txt");

  read(client_fd, buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST

START_TEST(test_tcp_connection_fails)
{
  struct TcpConnection *tcp_connection = NULL;
  memstub_failure_enable(0, 1);
  tcp_connection = tcp_connection_with_fd(fileno(stderr), logger, eloop);
  ck_assert(tcp_connection == NULL);
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
  tcase_add_test(tc, test_tcp_connection_sendfile);
  tcase_add_test(tc, test_tcp_connection_fails);
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
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

