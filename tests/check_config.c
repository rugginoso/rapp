#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config_private.h>
#include <logger.h>

#define S "sect"
#define N "name"
#define PS PARAM_STRING

#define ck_assert_call_ok(fun, ...) ck_assert_int_eq(fun(__VA_ARGS__), 0);
#define ck_assert_call_fail(fun, ...) ck_assert_int_eq(fun(__VA_ARGS__), 1);


struct Config *conf;

void
setup(void)
{
   struct Logger *logger = logger_open_console(LOG_LAST, stderr);
   conf = config_new(logger);
}

void
teardown(void)
{
    config_destroy(conf);
}


START_TEST(test_config_create)
{
    char *value;
    long v;
    ck_assert(conf != NULL);
    // missing required arguments
    ck_assert_call_fail(config_opt_add, NULL, S, N, PS, NULL);
    ck_assert_call_fail(config_opt_add, conf, NULL, N, PS, NULL);
    ck_assert_call_fail(config_opt_add, conf, S, NULL, PS, NULL);

    // setting properties on non-existing option
    ck_assert_call_fail(config_opt_set_range_int, conf, S, N, 0, 1);
    ck_assert_call_fail(config_opt_set_multivalued, conf, S, N, 0);
    ck_assert_call_fail(config_opt_set_default_int, conf, S, N, 1);
    ck_assert_call_fail(config_opt_set_default_string, conf, S, N, "a");
    ck_assert_call_fail(config_opt_set_default_string, conf, S, N, "a");
    // retrieving values from a non-existing option
    ck_assert_call_fail(config_get_nth_bool, conf, S, N, 0, (int *)&v);
    ck_assert_call_fail(config_get_nth_int, conf, S, N, 0, &v);
    ck_assert_call_fail(config_get_nth_string, conf, S, N, 0, &value);
    ck_assert(value == NULL);
    return;
}
END_TEST

START_TEST(test_config_opt_add)
{
    ck_assert_call_ok(config_opt_add, conf, S, N, PARAM_STRING, "tests");
}
END_TEST


static Suite *
config_suite(void)
{
  Suite *s = suite_create("rapp.core.config");
  TCase *tc = tcase_create("rapp.core.config");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_create);
  tcase_add_test(tc, test_config_opt_add);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = config_suite();
 SRunner *sr = srunner_create(s);
 srunner_set_log(sr, "check_config.log");
 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

