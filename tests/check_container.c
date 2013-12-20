/*
 * check_container.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>

#include <check.h>

#include "logger.h"
#include "container.h"

#include "test_dlstubs.h"


START_TEST(test_container_new_dlopen_fail)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
   struct Symbol syms[] = {
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_DLOPEN, syms);
  logger = logger_new_null();
  container = container_new(logger, "dummy", 0, NULL);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 0);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_dlsym_fail_get_abi_version)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_DLSYM },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  container = container_new(logger, "dummy", 0, NULL);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 0);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_abi_version_mismatch)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_PLUGIN },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  container = container_new(logger, "dummy", 0, NULL);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_create"), 0);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_dummy)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  container = container_new(logger, "dummy", 0, NULL);
  ck_assert(container != NULL);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_create"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_destroy"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_serve"), 1);
  container_destroy(container);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_create"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_destroy"), 1);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_serve_dummy)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  container = container_new(logger, "dummy", 0, NULL);
  ck_assert(container != NULL);
  container_serve(container,
                  (struct HTTPRequest *)syms,
                  (struct HTTPResponseWriter *)syms); /* FIXME */
  container_destroy(container);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_create"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_destroy"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_serve"), 1);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_logger_get)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  container = container_new(logger, "dummy", 0, NULL);
  ck_assert(container != NULL);
  ck_assert(dlstub_logger_get() == logger);
  container_destroy(container);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_null_new_destroy)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  logger = logger_new_null();
  container = container_new_null(logger, "test");
  ck_assert(container != NULL);
  container_destroy(container);
  logger_destroy(logger);
}
END_TEST

/* TODO: check logger output */
START_TEST(test_container_new_null_serve)
{
  int ret = 0;
  struct Logger *logger = NULL;
  struct Container *container = NULL;

  logger = logger_new_null();

  container = container_new_null(logger, "test");
  ck_assert(container != NULL);
  ret = container_serve(container,
                        (struct HTTPRequest *)logger,
                        (struct HTTPResponseWriter *)logger); /* FIXME */
  ck_assert_int_eq(ret, -1);

  container_destroy(container);
  logger_destroy(logger);
}
END_TEST


static Suite *
container_suite(void)
{
  Suite *s = suite_create("rapp.core.container");
  TCase *tc = tcase_create("rapp.core.container");

  tcase_add_test(tc, test_container_new_dlopen_fail);
  tcase_add_test(tc, test_container_new_dlsym_fail_get_abi_version);
  tcase_add_test(tc, test_container_new_abi_version_mismatch);
  tcase_add_test(tc, test_container_new_dummy);
  tcase_add_test(tc, test_container_serve_dummy);
  tcase_add_test(tc, test_container_logger_get);
  tcase_add_test(tc, test_container_new_null_new_destroy);
  tcase_add_test(tc, test_container_new_null_serve);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = container_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

