/*
 * RApp test container. Use this as template for new tests.
 * Do not forget to add the new test to tests/CMakeLists.txt!
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


static Suite *
container_suite(void)
{
  Suite *s = suite_create("rapp.core.container");
  TCase *tc = tcase_create("rapp.core.container");

  tcase_add_test(tc, test_container_new_dlopen_fail);
  tcase_add_test(tc, test_container_new_dlsym_fail_get_abi_version);
  tcase_add_test(tc, test_container_new_abi_version_mismatch);
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

