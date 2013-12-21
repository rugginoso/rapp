/*
 * check_httpresponsewriter.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <fcntl.h>

#include <check.h>

#include "eloop.h"
#include "tcpconnection.h"
#include "httpresponsewriter.h"
#include "test_utils.h"

#define HOST "localhost"
#define PORT 8000

#define MESSAGE "Hello world!"
#define MESSAGE_LEN STRLEN(MESSAGE)

struct ELoop *eloop = NULL;
struct TcpConnection *tcp_connection = NULL;
struct HTTPResponseWriter *response_writer = NULL;
int server_fd = -1;
int client_fd = -1;
int callback_called = 0;
char buf[MESSAGE_LEN];


static void
on_headers_sent(struct HTTPResponseWriter *response_writer,
                void                      *data)
{
  callback_called = 1;
}

static void
on_body_sent(struct HTTPResponseWriter *response_writer,
             void                      *data)
{
  callback_called = 1;
}

void
setup()
{
  callback_called = 0;
  eloop = event_loop_new();

  server_fd = listen_to(HOST, PORT);
  client_fd = connect_to(HOST, PORT);

  tcp_connection = tcp_connection_with_fd(accept(server_fd, NULL, NULL), eloop);

  response_writer = http_response_writer_new(tcp_connection, on_headers_sent, on_body_sent, NULL);
}

void
teardown()
{
  close(client_fd);
  close(server_fd);
  http_response_writer_destroy(response_writer);
  tcp_connection_destroy(tcp_connection);
  event_loop_destroy(eloop);
}

START_TEST(test_httpresponsewriter_headers_sent_callback_is_called_on_notify)
{
  http_response_writer_notify_headers_sent(response_writer);

  ck_assert_int_eq(callback_called, 1);
}
END_TEST

START_TEST(test_httpresponsewriter_body_sent_callback_is_called_on_notify)
{
  http_response_writer_notify_body_sent(response_writer);

  ck_assert_int_eq(callback_called, 1);
}
END_TEST

START_TEST(test_httpresponsewriter_write_data_writes_data)
{
  http_response_writer_write_data(response_writer, MESSAGE, MESSAGE_LEN);

  read(client_fd, buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST

START_TEST(test_httpresponsewriter_sendfile_writes_data)
{
  int file_fd = open("test_file.txt", O_WRONLY | O_CREAT, 0640);

  write(file_fd, MESSAGE, MESSAGE_LEN);
  close(file_fd);

  http_response_writer_sendfile(response_writer, "test_file.txt");

  read(client_fd, buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);

  unlink("test_file.txt");
}
END_TEST

START_TEST(test_httpresponsewriter_printf_writes_data)
{
  http_response_writer_printf(response_writer, "%s", MESSAGE);

  read(client_fd, buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST


static Suite *
httpresponsewriter_suite(void)
{
  Suite *s = suite_create("rapp.core.httpresponsewriter");
  TCase *tc = tcase_create("rapp.core.httpresponsewriter");

  tcase_add_checked_fixture(tc, setup, teardown);
  tcase_add_test(tc, test_httpresponsewriter_headers_sent_callback_is_called_on_notify);
  tcase_add_test(tc, test_httpresponsewriter_body_sent_callback_is_called_on_notify);
  tcase_add_test(tc, test_httpresponsewriter_write_data_writes_data);
  tcase_add_test(tc, test_httpresponsewriter_sendfile_writes_data);
  tcase_add_test(tc, test_httpresponsewriter_printf_writes_data);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = httpresponsewriter_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

