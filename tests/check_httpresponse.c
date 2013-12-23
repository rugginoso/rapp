/*
 * check_httpresponsewriter.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include <check.h>

#include "logger.h"
#include "httpresponse.h"

struct HTTPResponse *response = NULL;
struct Logger *logger = NULL;


void
setup()
{
  logger = logger_new_null();
  response = http_response_new(logger, "test");
}

void
teardown()
{
  http_response_destroy(response);
  logger_destroy(logger);
}

static char *
http_datetime()
{
  time_t now;
  char *datetime = malloc(32);

  time(&now);

  strftime(datetime, 32, "%a, %d %b %Y %H:%M:%S %z", gmtime(&now));

  return datetime;
}

START_TEST(test_httpresponse_append_data_writes_data)
{
  char *result = alloca(1024);
  ssize_t len = 0;

  http_response_append_data(response, "test", 4);

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  ck_assert_str_eq(result, "test");
}
END_TEST

START_TEST(test_httpresponse_write_header_correctly_formats_headers)
{
  char *result = alloca(1024);
  ssize_t len = 0;

  http_response_write_header(response, "key", "value");

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  ck_assert_str_eq(result, "key: value" HTTP_EOL);
}
END_TEST

START_TEST(test_httpresponse_end_headers_adds_server_date_empty_header)
{
  char *result = alloca(1024);
  char *datetime = NULL;
  char *expected = NULL;
  ssize_t len = 0;

  datetime = http_datetime();

  asprintf(&expected, "Server: test" HTTP_EOL
                      "Date: %s" HTTP_EOL
                      HTTP_EOL, datetime);
  free(datetime);

  http_response_end_headers(response);

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  ck_assert_str_eq(result, expected);

  free(expected);
}
END_TEST

START_TEST(test_httpresponse_read_data_supports_partials_reads)
{
  char *result = alloca(1024);
  ssize_t len = 0;

  http_response_append_data(response, "firstsecond", 11);

  len = http_response_read_data(response, result, 5);
  result[len] = 0;

  ck_assert_str_eq(result, "first");

  len = http_response_read_data(response, result, 6);
  result[len] = 0;

  ck_assert_str_eq(result, "second");
}
END_TEST

/* Coverage */
START_TEST(test_httpresponse_frees_not_consumed_data_on_destroy)
{
  http_response_append_data(response, "free me", 7);
}
END_TEST

START_TEST(test_httpresponse_write_status_line_appends_statusline)
{
  char *result = alloca(1024);
  ssize_t len = 0;

  http_response_write_status_line(response, "200 OK");

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  ck_assert_str_eq(result, "HTTP/1.1 200 OK" HTTP_EOL);
}
END_TEST

START_TEST(test_httpresponse_write_status_line_by_code_appends_statusline)
{
  char *result = alloca(1024);
  ssize_t len = 0;

  http_response_write_status_line_by_code(response, 200);

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  ck_assert_str_eq(result, "HTTP/1.1 200 OK" HTTP_EOL);
}
END_TEST

START_TEST(test_httpresponse_write_status_line_by_code_return_error_on_invalid_code)
{
  ck_assert_int_eq(http_response_write_status_line_by_code(response, 600), -1);
}
END_TEST

START_TEST(test_httpresponse_write_error_by_code_appends_error_page)
{
  char *result = alloca(1024);
  ssize_t len;
  char *datetime = NULL;
  char *expected = NULL;

  datetime = http_datetime();

  http_response_write_error_by_code(response, 404);

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  asprintf(&expected, "HTTP/1.1 404 Not Found"  HTTP_EOL \
                      "Content-Type: text/html" HTTP_EOL \
                      "Content-Length: 96"      HTTP_EOL \
                      "Server: test"            HTTP_EOL \
                      "Date: %s"                HTTP_EOL \
                      HTTP_EOL                           \
                      "<!DOCTYPE html>"                  \
                      "<html>"                           \
                        "<head>"                         \
                          "<title>Not Found</title>"     \
                        "</head>"                        \
                        "<body>"                         \
                          "<h1>Not Found</h1>"           \
                        "</body>"                        \
                      "</html>", datetime);
  free(datetime);

  ck_assert_str_eq(result, expected);
  free(expected);
}
END_TEST

static Suite *
httpresponse_suite(void)
{
  Suite *s = suite_create("rapp.core.httpresponse");
  TCase *tc = tcase_create("rapp.core.httpresponse");

  tcase_add_checked_fixture(tc, setup, teardown);
  tcase_add_test(tc, test_httpresponse_append_data_writes_data);
  tcase_add_test(tc, test_httpresponse_write_header_correctly_formats_headers);
  tcase_add_test(tc, test_httpresponse_end_headers_adds_server_date_empty_header);
  tcase_add_test(tc, test_httpresponse_read_data_supports_partials_reads);
  tcase_add_test(tc, test_httpresponse_frees_not_consumed_data_on_destroy);
  tcase_add_test(tc, test_httpresponse_write_status_line_appends_statusline);
  tcase_add_test(tc, test_httpresponse_write_status_line_by_code_appends_statusline);
  tcase_add_test(tc, test_httpresponse_write_status_line_by_code_return_error_on_invalid_code);
  tcase_add_test(tc, test_httpresponse_write_error_by_code_appends_error_page);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = httpresponse_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

