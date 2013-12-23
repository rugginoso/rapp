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
#include <config_private.h>
#include <logger.h>

#include "test_utils.h"
#define S "sectname"
#define N "optname"
#define PS PARAM_STRING


struct Config *conf;
char *empty[] = {"rapp", NULL};
char *usage[] = {"rapp", "--usage", NULL};
char *version[] = {"rapp", "--version", NULL};
char *help[] = {"rapp", "--help", NULL};

char**
copy_argv(int argc, char *argv[])
{
  char **res;
  int i;
  size_t len;
  res = calloc(argc, sizeof(char*));
  for (i=0; i<argc; i++) {
    if (argv[i]) {
      len = strlen(argv[i]) + 1;
      res[i] = malloc(len);
      strncpy(res[i], argv[i], len);
    }
  }
  return res;
}

void
clear_argv(int argc, char *argv[])
{
  int i;
  for (i=0; i<argc; i++) {
    if (argv[i])
      free(argv[i]);
  }
  free(argv);
}

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

void
test_loglevel(struct RappArguments *arguments, int pos,
              int argc, char **argv, const char *value, int expected)
{
  argv[pos] = strdup(value);
  ck_assert_call_ok(config_parse_early_commandline, arguments, 3, argv);
  ck_assert_int_eq(arguments->loglevel, expected);
  free(argv[pos]);
}

START_TEST(test_config_early_options)
{
  struct RappArguments arguments;
  char **args;
  char *loglevel[] = {"rapp", "--log-level", NULL, NULL};
  char *logout[] = {"rapp", "--log-output", "-", NULL};
  char *load[] = {"rapp", "--load", NULL, NULL};
  char *logcolor[] = {"rapp", "--log-nocolor"};
  ck_assert_call_fail(config_parse_early_commandline, &arguments, 0, 0);
  ck_assert_call_fail(config_parse_early_commandline, &arguments, 1, 0);
  ck_assert_call_ok(config_parse_early_commandline, &arguments, 1, empty);
  // defaults:
  ck_assert_int_eq(arguments.loglevel, LOG_INFO);
  ck_assert_int_eq(arguments.lognocolor, 0);
  ck_assert(arguments.logoutput == NULL);
  ck_assert(arguments.container == NULL);

  // argp arguments should not be defined (EINVAL)
  ck_assert_int_eq(config_parse_early_commandline(&arguments, 2, usage), EINVAL);
  ck_assert_int_eq(config_parse_early_commandline(&arguments, 2, version), EINVAL);
  ck_assert_int_eq(config_parse_early_commandline(&arguments, 2, help), EINVAL);

  // loglevel null arg
  ck_assert_call_fail(config_parse_early_commandline, &arguments, 3, loglevel);
  test_loglevel(&arguments, 2, 3, loglevel, "DEBUG", LOG_DEBUG);
  test_loglevel(&arguments, 2, 3, loglevel, "debug", LOG_DEBUG);
  test_loglevel(&arguments, 2, 3, loglevel, "info", LOG_INFO);
  test_loglevel(&arguments, 2, 3, loglevel, "warn", LOG_WARNING);
  test_loglevel(&arguments, 2, 3, loglevel, "error", LOG_ERROR);
  test_loglevel(&arguments, 2, 3, loglevel, "critical", LOG_CRITICAL);
  // unkown values defaults to LOG_INFO
  test_loglevel(&arguments, 2, 3, loglevel, "other", LOG_INFO);

  // TODO
  ck_assert_call_ok(config_parse_early_commandline, &arguments, 3, logout);

  ck_assert_call_ok(config_parse_early_commandline, &arguments, 2, logcolor);
  ck_assert_int_eq(arguments.lognocolor, 1);

  ck_assert_call_fail(config_parse_early_commandline, &arguments, 3, load);
  load[2] = strdup("path/to/container.so");
  ck_assert_call_ok(config_parse_early_commandline, &arguments, 3, load);
  ck_assert_str_eq(arguments.container, "path/to/container.so");
}
END_TEST

static Suite *
config_commandline_suite(void)
{
  Suite *s = suite_create("rapp.core.config_commandline");
  TCase *tc = tcase_create("rapp.core.config_commandline");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_early_options);
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
