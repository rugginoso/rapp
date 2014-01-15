/*
 * check_container.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>

#include <check.h>

#include "logger.h"
#include "container.h"
#include "config/common.h"

#include "test_dlstubs.h"
#include "test_memstubs.h"


START_TEST(test_container_new_dlopen_fail)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
   struct Symbol syms[] = {
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_DLOPEN, syms);
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new(logger, "dummy", config);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 0);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_dlsym_fail_get_abi_version)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_DLSYM },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new(logger, "dummy", config);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 0);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_abi_version_mismatch)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_PLUGIN },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new(logger, "dummy", config);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_create"), 0);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_setup"), 0);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_dummy)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { "rapp_setup",           DLSTUB_ERR_NONE },
    { "rapp_teardown",        DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new(logger, "dummy", config);
  ck_assert(container != NULL);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_create"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_destroy"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_serve"), 1);
//  ck_assert_int_eq(dlstub_get_lookup_count("rapp_setup"), 1);
//  ck_assert_int_eq(dlstub_get_lookup_count("rapp_teardown"), 1);
  container_destroy(container);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_dummy_memfail1)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { "rapp_setup",           DLSTUB_ERR_NONE },
    { "rapp_teardown",        DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  memstub_failure_enable(1, 1);
  container = container_new(logger, "dummy", config);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_create"), 1);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_new_dummy_memfail2)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { "rapp_setup",           DLSTUB_ERR_NONE },
    { "rapp_teardown",        DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  memstub_failure_enable(2, 1);
  container = container_new(logger, "dummy", config);
  ck_assert(container == NULL);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_lookup_count("rapp_create"), 1);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_serve_dummy)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { "rapp_setup",           DLSTUB_ERR_NONE },
    { "rapp_teardown",        DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new(logger, "dummy", config);
//  container_setup(container, config);
  ck_assert(container != NULL);
  container_serve(container,
                  (struct HTTPRequest *)syms,
                  (struct HTTPResponse *)syms); /* FIXME */
//  container_teardown/(container);
  container_destroy(container);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_get_abi_version"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_create"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_destroy"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_serve"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_setup"), 1);
  ck_assert_int_eq(dlstub_get_invoke_count("rapp_teardown"), 1);
  logger_destroy(logger);
}
END_TEST

START_TEST(test_container_logger_get)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;
  struct Symbol syms[] = {
    { "rapp_get_abi_version", DLSTUB_ERR_NONE },
    { "rapp_create",          DLSTUB_ERR_NONE },
    { "rapp_destroy",         DLSTUB_ERR_NONE },
    { "rapp_serve",           DLSTUB_ERR_NONE },
    { "rapp_setup",           DLSTUB_ERR_NONE },
    { "rapp_teardown",        DLSTUB_ERR_NONE },
    { NULL, 0 }
  };
  dlstub_setup(DLSTUB_ERR_NONE, syms);
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new(logger, "dummy", config);
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
  struct RappConfig *config = NULL;
  logger = logger_new_null();
  config = config_new(logger);
  container = container_new_null(logger, "test");
  ck_assert(container != NULL);
//  container_setup(container, config);
//  container_teardown(container);
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
                        (struct HTTPResponse *)logger); /* FIXME */
  ck_assert_int_eq(ret, -1);

  container_destroy(container);
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

static int
debug_setup(struct RappContainer *handle, const struct RappConfig *config)
{
  return 0;
}

static int
debug_teardown(struct RappContainer *handle)
{
  return 0;
}


START_TEST(test_container_custom_new_fail1)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;

  struct RappContainer debug_data = { 0, 0 };
  logger = logger_new_null();
  config = config_new(logger);
  memstub_failure_enable(1, 1);
  container = container_new_custom(logger, "debug", debug_setup, debug_teardown, debug_serve, debug_destroy, &debug_data);
  ck_assert(container == NULL);
}
END_TEST

START_TEST(test_container_custom_new_fail2)
{
  struct Logger *logger = NULL;
  struct Container *container = NULL;
  struct RappConfig *config = NULL;

  struct RappContainer debug_data = { 0, 0 };
  logger = logger_new_null();
  config = config_new(logger);
  memstub_failure_enable(2, 1);
  container = container_new_custom(logger, "debug", debug_setup, debug_teardown, debug_serve, debug_destroy, &debug_data);
  ck_assert(container == NULL);
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
  tcase_add_test(tc, test_container_custom_new_fail1);
  tcase_add_test(tc, test_container_custom_new_fail2);
  tcase_add_test(tc, test_container_new_dummy_memfail1);
  tcase_add_test(tc, test_container_new_dummy_memfail2);
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

