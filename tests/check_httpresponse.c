/*
 * check_httpresponsewriter.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <fcntl.h>

#include <check.h>

#include "httpresponse.h"

struct HTTPResponse *response = NULL;


void
setup()
{
  response = http_response_new();
}

void
teardown()
{
  http_response_destroy(response);
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

START_TEST(test_httpresponse_end_headers_adds_empty_header)
{
  char *result = alloca(1024);
  ssize_t len = 0;

  http_response_end_headers(response);

  len = http_response_read_data(response, result, 1024);
  result[len] = 0;

  ck_assert_str_eq(result, HTTP_EOL);
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

static Suite *
httpresponse_suite(void)
{
  Suite *s = suite_create("rapp.core.httpresponse");
  TCase *tc = tcase_create("rapp.core.httpresponse");

  tcase_add_checked_fixture(tc, setup, teardown);
  tcase_add_test(tc, test_httpresponse_append_data_writes_data);
  tcase_add_test(tc, test_httpresponse_write_header_correctly_formats_headers);
  tcase_add_test(tc, test_httpresponse_end_headers_adds_empty_header);
  tcase_add_test(tc, test_httpresponse_read_data_supports_partials_reads);
  tcase_add_test(tc, test_httpresponse_frees_not_consumed_data_on_destroy);
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

