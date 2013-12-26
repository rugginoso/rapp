/*
 * check_config_env.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <errno.h>
#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config/common.h>
#include <logger.h>

#include "test_utils.h"

struct Config *conf;

void
setup(void)
{
  clearenv();
  struct Logger *logger = logger_new_console(LOG_LAST, stderr);
  conf = config_new(logger);
}

void
teardown(void)
{
  config_destroy(conf);
}

START_TEST(test_config_env)
{
  char *value;
  ck_assert_call_fail(config_read_env, NULL);
  // conf should ignore non-related or not set options
  putenv("OTHER_VALUE=3");
  putenv("RAPP_ADDRESS=0.0.0.0");
  putenv("RAPP_MY_SECTION_MY_OPTION=yes");
  ck_assert_call_ok(config_read_env, conf);

  config_opt_add(conf, RAPP_CONFIG_SECTION, "address", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "other", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, "my_section", "my_option", PARAM_STRING, NULL, NULL);
  config_opt_set_default_string(conf, RAPP_CONFIG_SECTION, "address", "localhost");
  ck_assert_call_ok(config_read_env, conf);
  config_get_string(conf, RAPP_CONFIG_SECTION, "address", &value);
  ck_assert_str_eq(value, "0.0.0.0");
  config_get_string(conf, "my_section", "my_option", &value);
  ck_assert_str_eq(value, "yes");
}
END_TEST

START_TEST(test_config_env_override_conf)
{
  char *config_s = "---\ncore: {address: 127.0.0.1}";
  char *value;
  putenv("RAPP_ADDRESS=0.0.0.0");
  config_opt_add(conf, RAPP_CONFIG_SECTION, "address", PARAM_STRING, NULL, NULL);
  config_opt_set_default_string(conf, RAPP_CONFIG_SECTION, "address", "localhost");
  ck_assert_call_ok(config_read_env, conf);
  ck_assert_call_ok(config_parse_string, conf, config_s);
  config_get_string(conf, RAPP_CONFIG_SECTION, "address", &value);
  ck_assert_str_eq(value, "0.0.0.0");
}
END_TEST

START_TEST(test_config_env_overridden_by_commandline)
{
  char *cmdline[] = {"rapp", "--address", "127.0.0.1"};
  char *value;
  putenv("RAPP_ADDRESS=0.0.0.0");
  config_opt_add(conf, RAPP_CONFIG_SECTION, "address", PARAM_STRING, NULL, NULL);
  config_opt_set_default_string(conf, RAPP_CONFIG_SECTION, "address", "localhost");
  ck_assert_call_ok(config_parse_commandline, conf, 3, cmdline);
  ck_assert_call_ok(config_read_env, conf);
  config_get_string(conf, RAPP_CONFIG_SECTION, "address", &value);
  ck_assert_str_eq(value, "127.0.0.1");
}
END_TEST

START_TEST(test_config_env_multivalue)
{
  int value;
  // set a string that starts and ends with : as edge cases
  putenv("RAPP_SECTION_OPTION=:1:2:3:");
  config_opt_add(conf, "section", "option", PARAM_INT, NULL, NULL);
  config_opt_set_multivalued(conf, "section", "option", 1);
  ck_assert_call_ok(config_read_env, conf);
  config_get_num_values(conf, "section", "option", &value);
  ck_assert_int_eq(3, value);
  config_get_nth_int(conf, "section", "option", 0, (long *) &value);
  ck_assert_int_eq(1, value);
  config_get_nth_int(conf, "section", "option", 1, (long *) &value);
  ck_assert_int_eq(2, value);
  config_get_nth_int(conf, "section", "option", 2, (long *) &value);
  ck_assert_int_eq(3, value);
}
END_TEST

START_TEST(test_config_env_bool_ok)
{
  int value;
  putenv("RAPP_SECTION_OPTION=yes");
  config_opt_add(conf, "section", "option", PARAM_BOOL, NULL, NULL);
  ck_assert_call_ok(config_read_env, conf);
  config_get_bool(conf, "section", "option", &value);
  ck_assert_int_eq(value, 1);
}
END_TEST

START_TEST(test_config_env_bool_fail)
{
  int value;
  putenv("RAPP_SECTION_OPTION=2");
  config_opt_add(conf, "section", "option", PARAM_BOOL, NULL, NULL);
  ck_assert_call_ok(config_read_env, conf);
  ck_assert_call_fail(config_get_bool, conf, "section", "option", &value);
}
END_TEST

static Suite *
config_env_suite(void)
{
  Suite *s = suite_create("rapp.core.config_env");
  TCase *tc = tcase_create("rapp.core.config_env");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_env);
  tcase_add_test(tc, test_config_env_override_conf);
  tcase_add_test(tc, test_config_env_overridden_by_commandline);
  tcase_add_test(tc, test_config_env_multivalue);
  tcase_add_test(tc, test_config_env_bool_ok);
  tcase_add_test(tc, test_config_env_bool_fail);
  suite_add_tcase(s, tc);

  return s;
}


int
main (void)
{
  int number_failed = 0;

  Suite *s = config_env_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_log(sr, "check_config_env.log");
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
