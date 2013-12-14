#include <stdlib.h>

#include <check.h>

#include "httprequest.h"


struct HTTPRequest *http_request = NULL;
static int finish_called = 0;


void finish_func(struct HTTPRequest *request, void *data)
{
  finish_called = 1;
}

void setup()
{
  http_request = http_request_new();
  finish_called = 0;
}

void teardown()
{
  http_request_destroy(http_request);
}

START_TEST(test_httprequest_error_on_invalid_request)
{
  char *request = "casual data";

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), -1);
}
END_TEST

START_TEST(test_httprequest_calls_callback_when_finish_to_parse_request)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\n\r\n";

  http_request_set_parse_finish_callback(http_request, finish_func, NULL);

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), 0);

  ck_assert(finish_called == 1);
}
END_TEST

START_TEST(test_httprequest_gets_the_right_url)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange url_range;
  char *url = NULL;

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), 0);

  request_buffer = http_request_get_buffer(http_request);

  http_request_get_url_range(http_request, &url_range);
  ck_assert_uint_gt(url_range.length, 0);

  EXTRACT_MEMORY_RANGE(url, request_buffer, url_range);
  ck_assert_str_eq(url, "/hello/world/");
}
END_TEST

START_TEST(test_httprequest_gets_all_the_headers)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\nHost: someserver\r\nAccept: text/html\r\n\r\n";
  const char *request_buffer = NULL;
  struct HeaderMemoryRange *headers_ranges;
  unsigned n_headers = 0;
  char *host_header = NULL;
  char *accept_header = NULL;

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), 0);

  request_buffer = http_request_get_buffer(http_request);

  http_request_get_headers_ranges(http_request, &headers_ranges, &n_headers);

  ck_assert_uint_eq(n_headers, 2);

  EXTRACT_MEMORY_RANGE(host_header, request_buffer, headers_ranges[0].value);
  ck_assert_str_eq(host_header, "someserver");

  EXTRACT_MEMORY_RANGE(accept_header, request_buffer, headers_ranges[1].value);
  ck_assert_str_eq(accept_header, "text/html");
}
END_TEST

START_TEST(test_httprequest_gets_a_specific_header)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\nHost: someserver\r\nAccept: text/html\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange header_range;
  char *host_header = NULL;

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), 0);

  request_buffer = http_request_get_buffer(http_request);

  ck_assert_int_eq(http_request_get_header_value_range(http_request, "host", &header_range), 0);

  ck_assert_uint_gt(header_range.length, 0);

  EXTRACT_MEMORY_RANGE(host_header, request_buffer, header_range);
  ck_assert_str_eq(host_header, "someserver");
}
END_TEST

START_TEST(test_httprequest_error_on_not_existent_header)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\nHost: someserver\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange header_range;

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), 0);

  request_buffer = http_request_get_buffer(http_request);

  ck_assert_int_eq(http_request_get_header_value_range(http_request, "accept", &header_range), -1);
}
END_TEST

START_TEST(test_httprequest_returns_error_on_too_many_headers)
{
  char *request = strdup("GET /hello/world/ HTTP/1.1\r\n");
  char *header = NULL;
  int i = 0;

  while (i < (HTTP_REQUEST_MAX_HEADERS + 1)) {
    asprintf(&header, "X-HTTP-FOO%d: bar\r\n", i);
    request = realloc(request, strlen(request) + strlen(header) + 1);
    memcpy(&request[strlen(request)], header, strlen(header) + 1);
    free(header);
    i++;
  }

  ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), -1);

  free(request);
}
END_TEST

START_TEST(test_httprequest_gets_the_body)
{
  char *request = "POST /hello/world/ HTTP/1.1\r\nContent-Length: 12\r\n\r\nHello world!\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange body_range;
  char *body = NULL;

  http_request_append_data(http_request, request, strlen(request));

  request_buffer = http_request_get_buffer(http_request);

  http_request_get_body_range(http_request, &body_range);
  ck_assert_uint_gt(body_range.length, 0);

  EXTRACT_MEMORY_RANGE(body, request_buffer, body_range);
  ck_assert_str_eq(body, "Hello world!");
}
END_TEST

static Suite *
httprequest_suite(void)
{
  Suite *s = suite_create("rapp.core.httprequest");
  TCase *tc = tcase_create("rapp.core.httprequest");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_httprequest_error_on_invalid_request);
  tcase_add_test(tc, test_httprequest_calls_callback_when_finish_to_parse_request);
  tcase_add_test(tc, test_httprequest_gets_the_right_url);
  tcase_add_test(tc, test_httprequest_gets_all_the_headers);
  tcase_add_test(tc, test_httprequest_gets_a_specific_header);
  tcase_add_test(tc, test_httprequest_error_on_not_existent_header);
  tcase_add_test(tc, test_httprequest_returns_error_on_too_many_headers);
  tcase_add_test(tc, test_httprequest_gets_the_body);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = httprequest_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

