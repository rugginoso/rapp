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
#include "logger.h"


START_TEST(test_httprouter_new_destroy)
{
  struct Logger *logger = NULL;
  struct HTTPRouter *router = NULL;
  logger = logger_new_null();
  router = http_router_new(logger, ROUTE_MATCH_FIRST);
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

  router = http_router_new(logger, ROUTE_MATCH_FIRST);
  ck_assert(router != NULL);

  ret = http_router_serve(router, (struct HTTPRequest *)&logger, (struct HTTPResponse *)&logger); /* FIXME */
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
            struct HTTPResponse       *response)
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

  router = http_router_new(logger, ROUTE_MATCH_FIRST);
  ck_assert(router != NULL);
  ret = http_router_set_default_container(router, debug);
  ck_assert_int_eq(ret, 0);

  ret = http_router_serve(router, (struct HTTPRequest *)&debug_data, (struct HTTPResponse *)&debug_data); /* FIXME */
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

  router = http_router_new(logger, ROUTE_MATCH_FIRST);
  ck_assert(router != NULL);
  ret = http_router_bind(router, route, debug);
  ck_assert_int_eq(ret, 0);

  request = http_request_new_fake_url(logger, route);

  ret = http_router_serve(router, request, (struct HTTPResponse *)&debug_data); /* FIXME */
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

#define ROUTES_NUM_SMALL  12
#define ROUTES_NUM_BIG    1024

static void
check_many_routes(const char *route_tmpl,
                  int         routes_num)
{
  int i = 0;
  int ret = 0;
  char buf[128] = { '\0' };
  struct Logger *logger = NULL;
  struct HTTPRequest *request = NULL;
  struct HTTPRouter *router = NULL;
  struct Container **debug = NULL;
  struct RappContainer *debug_data = NULL;

  debug = calloc(routes_num, sizeof(struct Container *) * routes_num);
  ck_assert(debug != NULL);
  debug_data = calloc(routes_num, sizeof(struct RappContainer) * routes_num);
  ck_assert(debug_data != NULL);

  logger = logger_new_null();
  router = http_router_new(logger, ROUTE_MATCH_FIRST);
  ck_assert(router != NULL);

  /* setup */
  for (i = 0; i < routes_num; i++) {
    snprintf(buf, sizeof(buf), "debug#%02i", i);
    debug[i] = container_new_custom(logger, buf, debug_serve, debug_destroy, &(debug_data[i]));

    snprintf(buf, sizeof(buf), route_tmpl, 5000+i);
    ret = http_router_bind(router, buf, debug[i]);
    ck_assert_int_eq(ret, 0);
  }

  /* exercise */
  for (i = 0; i < routes_num; i++) {
    snprintf(buf, sizeof(buf), route_tmpl, 5000+i);
    request = http_request_new_fake_url(logger, buf);

    ret = http_router_serve(router, request,
                            (struct HTTPResponse *)request); /* FIXME */
    ck_assert_int_eq(ret, 0);
    http_request_destroy(request);
  }

  /* verify and clean */
  for (i = 0; i < routes_num; i++) {
    ck_assert_int_eq(debug_data[i].invoke_count, 1);
    container_destroy(debug[i]);
  }

  http_router_destroy(router);
  logger_destroy(logger);
  free(debug_data);
  free(debug);
}

START_TEST(test_httproter_many_small_routes_num_small)
{
  check_many_routes("/%02i", ROUTES_NUM_SMALL);
}
END_TEST

START_TEST(test_httproter_many_small_routes_num_big)
{
  check_many_routes("/%02i", ROUTES_NUM_BIG);
}
END_TEST

START_TEST(test_httproter_many_long_routes_num_small)
{
  check_many_routes("/very/long/route/name/number%02i/to/properlyexerciseall/the_code", ROUTES_NUM_SMALL);
}
END_TEST

START_TEST(test_httproter_many_long_routes_num_big)
{
  check_many_routes("/very/long/route/name/number%02i/to/properlyexerciseall/the_code", ROUTES_NUM_BIG);
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
  tcase_add_test(tc, test_httproter_many_small_routes_num_small);
  tcase_add_test(tc, test_httproter_many_small_routes_num_big);
  tcase_add_test(tc, test_httproter_many_long_routes_num_small);
  tcase_add_test(tc, test_httproter_many_long_routes_num_big);
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

