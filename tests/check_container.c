/*
 * RApp test container. Use this as template for new tests.
 * Do not forget to add the new test to tests/CMakeLists.txt!
 */

#include <stdlib.h>

#include <check.h>

#include "logger.h"
#include "container.h"


START_TEST(test_container_dlopen_fail)
{
  struct Logger *logger = logger_new_null();
  /* TODO: check logger output! */
  struct Container *container = container_new(logger, "NULL", 0, NULL);
  ck_assert(container == NULL);
  logger_destroy(logger);
}
END_TEST


static Suite *
container_suite(void)
{
  Suite *s = suite_create("rapp.core.container");
  TCase *tc = tcase_create("rapp.core.container");

  tcase_add_test(tc, test_container_dlopen_fail);
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

