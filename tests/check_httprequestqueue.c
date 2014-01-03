/*
 * check_httprequestqueue.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>

#include <check.h>

#include "logger.h"
#include "httprequestqueue.h"
#include "httprequest.h"


struct Logger *logger = NULL;
struct HTTPRequestQueue *queue = NULL;

void setup()
{
  logger = logger_new_null();
  queue = http_request_queue_new(logger);
}

void teardown()
{
  http_request_queue_destroy(queue);
  logger_destroy(logger);
}

void
new_request_func(struct HTTPRequestQueue *q,
                 void                    *data)
{
  (*(int *)(data)) = 1;
}

START_TEST(test_httprequestqueue_error_on_invalid_request)
{
  char *request = "casual data";

  ck_assert_int_eq(http_request_queue_append_data(queue, request, strlen(request)), -1);
}
END_TEST

START_TEST(test_httprequestqueue_calls_callback_when_new_request_is_processed)
{
  int data = 0;
  char *request = "GET /hello/world/ HTTP/1.1\r\n\r\n";

  http_request_queue_set_new_request_callback(queue, new_request_func, &data);

  http_request_queue_append_data(queue, request, strlen(request));

  ck_assert(data == 1);
}
END_TEST

START_TEST(test_httprequestqueue_returns_error_on_too_many_headers)
{
  char *request = strdup("GET /hello/world/ HTTP/1.1\r\n");
  char *header = NULL;
  int i = 0;
  int append_data_return = 0;

  while (i < (HTTP_REQUEST_MAX_HEADERS + 1)) {
    asprintf(&header, "X-HTTP-FOO%d: bar\r\n", i);
    request = realloc(request, strlen(request) + strlen(header) + 1);
    memcpy(&request[strlen(request)], header, strlen(header) + 1);
    free(header);
    i++;
  }

  append_data_return = http_request_queue_append_data(queue, request, strlen(request));

  ck_assert_int_eq(append_data_return, -1);

  free(request);
}
END_TEST


static Suite *
httprequestqueue_suite(void)
{
  Suite *s = suite_create("rapp.core.httprequestqueue");
  TCase *tc = tcase_create("rapp.core.httprequestqueue");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_httprequestqueue_error_on_invalid_request);
  tcase_add_test(tc, test_httprequestqueue_calls_callback_when_new_request_is_processed);
  tcase_add_test(tc, test_httprequestqueue_returns_error_on_too_many_headers);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = httprequestqueue_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

