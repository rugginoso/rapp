/*
 * check_httprequest.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>

#include <check.h>

#include "logger.h"
#include "httprequestqueue.h"
#include "httprequest.h"


struct HTTPRequest *http_request = NULL;
struct HTTPRequestQueue *queue = NULL;
struct Logger *logger = NULL;
static const char *HTTP_METHODS_FIXTURES[HTTP_METHOD_MAX] = {
  "DELETE",
  "GET",
  "HEAD",
  "POST",
  "PUT",
  "CONNECT",
  "OPTIONS",
  "TRACE",
  "COPY",
  "LOCK",
  "MKCOL",
  "MOVE",
  "PROPFIND",
  "PROPPATCH",
  "SEARCH",
  "UNLOCK",
  "REPORT",
  "MKACTIVITY",
  "CHECKOUT",
  "MERGE",
  "M-SEARCH",
  "NOTIFY",
  "SUBSCRIBE",
  "UNSUBSCRIBE",
  "PATCH",
  "PURGE"
};


void setup()
{
  logger = logger_new_null();
  queue = http_request_queue_new(logger);
}

void teardown()
{
  http_request_destroy(http_request);
  http_request_queue_destroy(queue);
  logger_destroy(logger);
}

/*START_TEST(test_httprequest_error_on_invalid_request)*/
/*{*/
  /*char *request = "casual data";*/

  /*ck_assert_int_eq(http_request_append_data(http_request, request, strlen(request)), -1);*/
/*}*/
/*END_TEST*/

/*START_TEST(test_httprequest_calls_callback_when_finish_to_parse_request)*/
/*{*/
  /*char *request = "GET /hello/world/ HTTP/1.1\r\n\r\n";*/

  /*http_request_set_parse_finish_callback(http_request, finish_func, NULL);*/

  /*http_request_append_data(http_request, request, strlen(request));*/

  /*ck_assert(finish_called == 1);*/
/*}*/
/*END_TEST*/

START_TEST(test_httprequest_gets_the_right_method)
{
  char *request = NULL;
  int i = 0;
  int append_data_return = 0;

  while (i < HTTP_METHOD_MAX) {
    asprintf(&request, "%s /hello/world/ HTTP/1.1\r\n\r\n", HTTP_METHODS_FIXTURES[i]);
    append_data_return = http_request_queue_append_data(queue, request, strlen(request));
    free(request);

    ck_assert_int_eq(append_data_return, 0);

    free(http_request);
    http_request = http_request_queue_get_next_request(queue);

    ck_assert(http_request != NULL);
    ck_assert_int_eq(http_request_get_method(http_request), i);

    i++;
  }
}
END_TEST

START_TEST(test_httprequest_gets_the_right_url)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange url_range;
  char *url = NULL;

  http_request_queue_append_data(queue, request, strlen(request));

  http_request = http_request_queue_get_next_request(queue);
  ck_assert(http_request != NULL);

  request_buffer = http_request_get_headers_buffer(http_request);

  http_request_get_url_range(http_request, &url_range);
  ck_assert(url_range.length > 0);

  EXTRACT_MEMORY_RANGE(url, request_buffer, url_range);
  ck_assert_str_eq(url, "/hello/world/");
}
END_TEST

START_TEST(test_httprequest_gets_the_right_url_field)
{
  char *request = "GET http://user:pass@example.com:8080/hello/?q=greeting#world HTTP/1.1\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange field_range;
  char *field = NULL;
  int i = 0;
  char *expected[HTTP_URL_FIELD_MAX] = {
    "http",
    "example.com",
    "8080",
    "/hello/",
    "q=greeting",
    "world",
    "user:pass",
  };

  http_request_queue_append_data(queue, request, strlen(request));

  http_request = http_request_queue_get_next_request(queue);
  ck_assert(http_request != NULL);

  request_buffer = http_request_get_headers_buffer(http_request);

  while (i < HTTP_URL_FIELD_MAX) {
    ck_assert_int_eq(http_request_get_url_field_range(http_request, i, &field_range), 0);
    ck_assert(field_range.length > 0);

    EXTRACT_MEMORY_RANGE(field, request_buffer, field_range);
    field[field_range.length] = 0;

    ck_assert_str_eq(field, expected[i]);

    i++;
  }
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

  http_request_queue_append_data(queue, request, strlen(request));

  http_request = http_request_queue_get_next_request(queue);
  ck_assert(http_request != NULL);

  request_buffer = http_request_get_headers_buffer(http_request);

  http_request_get_headers_ranges(http_request, &headers_ranges, &n_headers);

  ck_assert_int_eq(n_headers, 2);

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

  http_request_queue_append_data(queue, request, strlen(request));

  http_request = http_request_queue_get_next_request(queue);
  ck_assert(http_request != NULL);

  request_buffer = http_request_get_headers_buffer(http_request);

  ck_assert_int_eq(http_request_get_header_value_range(http_request, "host", &header_range), 0);

  ck_assert(header_range.length > 0);

  EXTRACT_MEMORY_RANGE(host_header, request_buffer, header_range);
  ck_assert_str_eq(host_header, "someserver");
}
END_TEST

START_TEST(test_httprequest_error_on_not_existent_header)
{
  char *request = "GET /hello/world/ HTTP/1.1\r\nHost: someserver\r\n\r\n";
  const char *request_buffer = NULL;
  struct MemoryRange header_range;

  http_request_queue_append_data(queue, request, strlen(request));

  http_request = http_request_queue_get_next_request(queue);
  ck_assert(http_request != NULL);

  request_buffer = http_request_get_headers_buffer(http_request);

  ck_assert_int_eq(http_request_get_header_value_range(http_request, "accept", &header_range), -1);
}
END_TEST

/*START_TEST(test_httprequest_returns_error_on_too_many_headers)*/
/*{*/
  /*char *request = strdup("GET /hello/world/ HTTP/1.1\r\n");*/
  /*char *header = NULL;*/
  /*int i = 0;*/
  /*int append_data_return = 0;*/

  /*while (i < (HTTP_REQUEST_MAX_HEADERS + 1)) {*/
    /*asprintf(&header, "X-HTTP-FOO%d: bar\r\n", i);*/
    /*request = realloc(request, strlen(request) + strlen(header) + 1);*/
    /*memcpy(&request[strlen(request)], header, strlen(header) + 1);*/
    /*free(header);*/
    /*i++;*/
  /*}*/

  /*append_data_return = http_request_append_data(http_request, request, strlen(request));*/

  /*ck_assert_int_eq(append_data_return, -1);*/

  /*free(request);*/
/*}*/
/*END_TEST*/

START_TEST(test_httprequest_gets_the_body)
{
  char *request = "POST /hello/world/ HTTP/1.1\r\nContent-Length: 12\r\n\r\nHello world!\r\n\r\n";
  const char *body = NULL;

  http_request_queue_append_data(queue, request, strlen(request));

  http_request = http_request_queue_get_next_request(queue);
  ck_assert(http_request != NULL);

  body = http_request_get_body(http_request);
  ck_assert_str_eq(body, "Hello world!");
}
END_TEST

static Suite *
httprequest_suite(void)
{
  Suite *s = suite_create("rapp.core.httprequest");
  TCase *tc = tcase_create("rapp.core.httprequest");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_httprequest_gets_the_right_method);
  tcase_add_test(tc, test_httprequest_gets_the_right_url);
  tcase_add_test(tc, test_httprequest_gets_the_right_url_field);
  tcase_add_test(tc, test_httprequest_gets_all_the_headers);
  tcase_add_test(tc, test_httprequest_gets_a_specific_header);
  tcase_add_test(tc, test_httprequest_error_on_not_existent_header);
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

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

