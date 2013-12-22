/*
 * check_httprouter.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>

#include <check.h>

#include "container.h"
#include "logger.h"
#include "httprequest.h"
#include "httprouter.h"


START_TEST(test_httprouter_new_destroy)
{
  struct Logger *logger = NULL;
  struct HTTPRouter *router = NULL;
  logger = logger_new_null();
  router = http_router_new(logger);
  ck_assert(router != NULL);
  http_router_destroy(router);
  logger_destroy(logger);
}
END_TEST

/* TODO: check the logger output */
START_TEST(test_httprouter_serve_without_containers)
{
  int ret = 0;
  struct Logger *logger = NULL;
  struct HTTPRouter *router = NULL;

  logger = logger_new_null();

  router = http_router_new(logger);
  ck_assert(router != NULL);

  ret = http_router_serve(router, (struct HTTPRequest *)&logger, (struct HTTPResponseWriter *)&logger); /* FIXME */
  ck_assert_int_eq(ret, -1);

  http_router_destroy(router);
  logger_destroy(logger);
}
END_TEST


struct RappContainer {
  int invoke_count;
  int serve_ret;
};

static int
debug_serve(struct RappContainer      *handle,
            struct HTTPRequest        *http_request,
            struct HTTPResponseWriter *response_writer)
{
  handle->invoke_count++;
  return handle->serve_ret;
}

static int
debug_destroy(struct RappContainer *handle)
{
  return 0;
}

START_TEST(test_httprouter_default_container_alone)
{
  int ret = 0;
  struct Logger *logger = NULL;
  struct HTTPRouter *router = NULL;
  struct Container *debug = NULL;

  struct RappContainer debug_data = { 0, 0 };
  logger = logger_new_null();
  debug = container_new_custom(logger, "debug", debug_serve, debug_destroy, &debug_data);

  router = http_router_new(logger);
  ck_assert(router != NULL);
  ret = http_router_set_default_container(router, debug);
  ck_assert_int_eq(ret, 0);

  ret = http_router_serve(router, (struct HTTPRequest *)&debug_data, (struct HTTPResponseWriter *)&debug_data); /* FIXME */
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(debug_data.invoke_count, 1);

  container_destroy(debug);
  http_router_destroy(router);
  logger_destroy(logger);
}
END_TEST


static void
check_route(const char *route)
{
  int ret = 0;
  struct Logger *logger = NULL;
  struct HTTPRequest *request = NULL;
  struct HTTPRouter *router = NULL;
  struct Container *debug = NULL;

  struct RappContainer debug_data = { 0, 0 };
  logger = logger_new_null();
  debug = container_new_custom(logger, "debug", debug_serve, debug_destroy, &debug_data);

  router = http_router_new(logger);
  ck_assert(router != NULL);
  ret = http_router_bind(router, route, debug);
  ck_assert_int_eq(ret, 0);

  request = http_request_new_fake_url(route);

  ret = http_router_serve(router, request, (struct HTTPResponseWriter *)&debug_data); /* FIXME */
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(debug_data.invoke_count, 1);

  http_request_destroy(request);

  container_destroy(debug);
  http_router_destroy(router);
  logger_destroy(logger);
}

START_TEST(test_httprouter_one_route_short)
{
  check_route("/");
}
END_TEST

START_TEST(test_httprouter_one_route_long)
{
  check_route("/this/route/must/be/longer/than/32/characters");
}
END_TEST

static Suite *
httprouter_suite(void)
{
  Suite *s = suite_create("rapp.core.httprouter");
  TCase *tc = tcase_create("rapp.core.httprouter");

  tcase_add_test(tc, test_httprouter_new_destroy);
  tcase_add_test(tc, test_httprouter_serve_without_containers);
  tcase_add_test(tc, test_httprouter_default_container_alone);
  tcase_add_test(tc, test_httprouter_one_route_short);
  tcase_add_test(tc, test_httprouter_one_route_long);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = httprouter_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

