/*
 * check_config.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config/common.h>
#include <logger.h>

#include "test_utils.h"
#define S "sectname"
#define N "optname"
#define PS PARAM_STRING


struct RappConfig *conf;

void
setup(void)
{
  struct Logger *logger = logger_new_null();
  conf = config_new(logger);
}

void
teardown(void)
{
  config_destroy(conf);
}

struct ConfigOption*
get_test_option(void) {
  struct ConfigSection *sect;
  struct ConfigOption *opt;
  sect = get_section(conf, S);
  for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
    if (strcmp(opt->name, N) == 0)
      return opt;
  }
  return NULL;
}


START_TEST(test_config_create)
{
  char *value;
  long v;
  config_destroy(conf);
  setup();
  ck_assert(conf != NULL);
  // missing required arguments
  ck_assert_call_fail(rapp_config_opt_add, NULL, S, N, PS, NULL, NULL);
  ck_assert_call_fail(rapp_config_opt_add, conf, NULL, N, PS, NULL, NULL);
  ck_assert_call_fail(rapp_config_opt_add, conf, S, NULL, PS, NULL, NULL);
  struct ConfigSection *sect = get_section(conf, NULL);
  ck_assert(sect == NULL);

  // setting properties on non-existing option
  ck_assert_call_fail(rapp_config_opt_set_range_int, conf, S, N, 0, 1);
  ck_assert_call_fail(rapp_config_opt_set_multivalued, conf, S, N, 0);
  ck_assert_call_fail(rapp_config_opt_set_default_int, conf, S, N, 1);
  ck_assert_call_fail(rapp_config_opt_set_default_string, conf, S, N, "a");
  ck_assert_call_fail(rapp_config_opt_set_default_string, conf, S, N, "a");
  // retrieving values from a non-existing option
  ck_assert_call_fail(rapp_config_get_nth_bool, conf, S, N, 0, (int *)&v);
  ck_assert_call_fail(rapp_config_get_nth_int, conf, S, N, 0, &v);
  ck_assert_call_fail(rapp_config_get_nth_string, conf, S, N, 0, &value);
  ck_assert(value == NULL);

  // wrong/accepted names
  ck_assert_call_fail(rapp_config_opt_add, conf, S, "-test", PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, S, "test-", PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, S, "test-opt", PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, S, "_test", PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, S, "test_", PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, S, "test.name", PS, 0, 0);
  ck_assert_call_ok(rapp_config_opt_add, conf, S, "test_name", PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, "-test", N, PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, "test-", N, PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, "_test", N, PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, "test_", N, PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, "test.name", N, PS, 0, 0);
  ck_assert_call_ok(rapp_config_opt_add, conf, "test_name", N, PS, 0, 0);
  ck_assert_call_fail(rapp_config_opt_add, conf, "test-name", N, PS, 0, 0);
  return;
}
END_TEST

START_TEST(test_config_opt_string)
{
  struct ConfigOption *opt;
  char *value;
  int res;
  ck_assert_call_ok(rapp_config_opt_add, conf, S, N, PARAM_STRING, "tests", "META");
  opt = get_test_option();
  ck_assert_str_eq(opt->metavar, "META");
  ck_assert_call_fail(rapp_config_opt_set_default_string, conf, S, N, NULL);
  ck_assert_call_ok(rapp_config_opt_set_default_string, conf, S, N, "default");
  ck_assert_call_fail(rapp_config_opt_set_range_int, conf, S, N, 0, 1);
  ck_assert_str_eq(opt->default_value.strvalue, "default");
  ck_assert_call_fail(rapp_config_get_nth_string, conf, S, N, 1, &value);
  ck_assert_call_fail(rapp_config_get_nth_int, conf, S, N, 0, (long *)&res);
  ck_assert_call_ok(rapp_config_get_nth_string, conf, S, N, 0, &value);
  ck_assert_str_eq(value, "default");
  free(value);

  ck_assert_call_fail(config_add_value_int, conf, S, N, 4);
  ck_assert_call_ok(config_add_value_string, conf, S, N, "value");
  // multivalued is off
  ck_assert_call_fail(config_add_value_string, conf, S, N, "value");
  ck_assert_call_ok(rapp_config_get_nth_string, conf, S, N, 0, &value);
  ck_assert_str_eq(value, "value");
  free(value);
  ck_assert_call_ok(rapp_config_opt_set_multivalued, conf, S, N, 1);
  ck_assert_call_ok(config_add_value_string, conf, S, N, "value2");
  ck_assert_call_ok(rapp_config_get_num_values, conf, S, N, &res);
  ck_assert_int_eq(res, 2);
  ck_assert_call_ok(rapp_config_get_nth_string, conf, S, N, 0, &value);
  ck_assert_str_eq(value, "value");
  free(value);
  ck_assert_call_ok(rapp_config_get_nth_string, conf, S, N, 1, &value);
  ck_assert_str_eq(value, "value2");
  free(value);
  ck_assert_call_fail(rapp_config_get_nth_string, conf, S, N, -1, &value);
  ck_assert_call_fail(rapp_config_get_nth_string, conf, S, N, 2, &value);
  return;
}
END_TEST

START_TEST(test_config_opt_int)
{
  struct ConfigOption *opt;
  long res;
  int ires;
  ck_assert_call_ok(rapp_config_opt_add, conf, S, N, PARAM_INT, "tests", NULL);
  opt = get_test_option();
  ck_assert_call_ok(rapp_config_opt_set_default_int, conf, S, N, 3);
  ck_assert_int_eq(opt->default_value.intvalue, 3);
  ck_assert_call_ok(rapp_config_get_nth_int, conf, S, N, 0, &res);
  ck_assert_int_eq(res, 3);
  ck_assert_call_fail(rapp_config_get_nth_int, conf, S, N, 1, &res);

  ck_assert_call_fail(config_add_value_string, conf, S, N, "value");
  ck_assert_call_ok(config_add_value_int, conf, S, N, 2);
  ck_assert_call_fail(rapp_config_get_nth_int, conf, S, N, 1, &res);
  // multivalued is off
  ck_assert_call_fail(config_add_value_int, conf, S, N, 1);
  ck_assert_call_ok(rapp_config_get_nth_int, conf, S, N, 0, &res);
  ck_assert_int_eq(res, 2);

  ck_assert_call_fail(rapp_config_opt_set_range_int, conf, S, N, 1, 1);
  ck_assert_call_fail(rapp_config_opt_set_range_int, conf, S, N, 1, 0);
  ck_assert_call_ok(rapp_config_opt_set_range_int, conf, S, N, 0, 10);

  ck_assert_call_ok(rapp_config_opt_set_multivalued, conf, S, N, 1);
  ck_assert_call_fail(config_add_value_int, conf, S, N, -1);
  ck_assert_call_fail(config_add_value_int, conf, S, N, 11);
  ck_assert_call_ok(config_add_value_int, conf, S, N, 0);
  ck_assert_call_ok(config_add_value_int, conf, S, N, 10);
  ck_assert_call_ok(rapp_config_get_num_values, conf, S, N, &ires);
  ck_assert_int_eq(ires, 3);
  ck_assert_call_ok(rapp_config_get_nth_int, conf, S, N, 1, &res);
  ck_assert_int_eq(res, 0);
  ck_assert_call_ok(rapp_config_get_nth_int, conf, S, N, 2, &res);
  ck_assert_int_eq(res, 10);
  ck_assert_call_fail(rapp_config_get_nth_int, conf, S, N, -1, &res);
  ck_assert_call_fail(rapp_config_get_nth_int, conf, S, N, 3, &res);
  return;
}
END_TEST

START_TEST(test_config_opt_bool)
{
  int b;
  long l;
  struct ConfigOption *opt;
  ck_assert_call_ok(rapp_config_opt_add, conf, S, N, PARAM_BOOL, "tests", "META");
  opt = get_test_option();
  ck_assert(opt->metavar == NULL);
  // autorange for booleans
  ck_assert_call_fail(config_add_value_int, conf, S, N, -1);
  ck_assert_call_fail(config_add_value_int, conf, S, N, 2);
  ck_assert_call_fail(rapp_config_opt_set_default_bool, conf, S, N, 2);
  ck_assert_call_fail(rapp_config_opt_set_default_bool, conf, S, N, -1);
  ck_assert_call_ok(rapp_config_opt_set_default_bool, conf, S, N, 1);
  ck_assert_call_ok(rapp_config_get_nth_int, conf, S, N, 0, &l);
  ck_assert_int_eq(1, l);
  ck_assert_call_ok(rapp_config_get_nth_bool, conf, S, N, 0, &b);
  ck_assert_int_eq(b, 1);
  return;
}
END_TEST

  static Suite *
config_suite(void)
{
  Suite *s = suite_create("rapp.core.config");
  TCase *tc = tcase_create("rapp.core.config");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_create);
  tcase_add_test(tc, test_config_opt_string);
  tcase_add_test(tc, test_config_opt_int);
  tcase_add_test(tc, test_config_opt_bool);
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
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
