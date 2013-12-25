/*
 * check_config_commandline.c - is part of RApp.
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
  struct Logger *logger = logger_new_null();
  conf = config_new(logger);
}

void
teardown(void)
{
  config_destroy(conf);
}

START_TEST(test_config_env)
{
  ck_assert_call_ok(config_read_env, conf, NULL);
}
END_TEST

static Suite *
config_commandline_suite(void)
{
  Suite *s = suite_create("rapp.core.config_env");
  TCase *tc = tcase_create("rapp.core.config_env");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_env);
  suite_add_tcase(s, tc);

  return s;
}


int
main (void)
{
  int number_failed = 0;

  Suite *s = config_commandline_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_log(sr, "check_config_commandline.log");
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
