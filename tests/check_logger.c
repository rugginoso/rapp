/*
 * RApp test logger. Use this as template for new tests.
 * Do not forget to add the new test to tests/CMakeLists.txt!
 */

#include <stdlib.h>

#include <check.h>

#include <logger.h>


START_TEST(test_logger_null)
{
  int err = 0;
  struct Logger *logger = logger_open_null();
  ck_assert(logger != NULL);
  err = logger_trace(logger, LOG_CRITICAL, __FILE__, "this is going to be discarded!");
  ck_assert_int_eq(err, 0);
  err = logger_flush(logger);
  ck_assert_int_eq(err, 0);
  logger_destroy(logger);
}
END_TEST


static Suite *
logger_suite(void)
{
  Suite *s = suite_create("rapp.core.logger");
  TCase *tc = tcase_create("rapp.core.logger");

  tcase_add_test(tc, test_logger_null);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = logger_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

