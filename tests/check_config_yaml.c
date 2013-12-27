/*
 * check_config_yaml.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config/common.h>
#include <rapp/rapp_config.h>
#include <logger.h>

#include "test_utils.h"
#include "yaml_documents.h"

#define CHECK_STR(CONF, SECT, NAME, STR, EXPECTED, POS)         \
  do {                                                          \
    config_get_nth_string(CONF, SECT, NAME, POS, &STR);         \
    ck_assert_str_eq(STR, EXPECTED);                            \
    free(STR);                                                  \
  } while(0)

#define CHECK_INT(CONF, SECT, NAME, VAL, EXPECTED, POS)         \
  do {                                                          \
    config_get_nth_int(CONF, SECT, NAME, POS, &VAL);            \
    ck_assert_int_eq(VAL, EXPECTED);                            \
  } while(0)

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

START_TEST(test_config_yaml_parse)
{
  long vi;
  char *vs;
  const char *c = "core";
  struct Logger *logger = logger_new_null();
  struct Config *conf2 = config_new(logger);
  struct Config *cp;
  const char *yaml;
  int i;

  for (i=0; i<2; i++) {
    if (i == 0) {
      cp = conf;
      yaml = yaml_good;
    } else {
      cp = conf2;
      yaml = yaml_good_inline;
    }
    config_opt_add(cp, c, "address", PARAM_STRING, NULL, NULL);
    config_opt_add(cp, c, "port", PARAM_INT, NULL, NULL);
    config_opt_set_range_int(cp, c, "port", 1, 65535);
    config_opt_add(cp, c, "test_list", PARAM_STRING, NULL, NULL);
    config_opt_add(cp, c, "test_list_inline", PARAM_INT, NULL, NULL);
    config_opt_add(cp, c, "test_bool", PARAM_BOOL, NULL, NULL);
    config_opt_set_multivalued(cp, c, "test_list", 1);
    config_opt_set_multivalued(cp, c, "test_list_inline", 1);
    ck_assert_call_ok(config_parse_string, cp, yaml);
    CHECK_STR(cp, c, "address", vs, "127.0.0.1", 0);
    CHECK_INT(cp, c, "port", vi, 8080, 0);
    CHECK_STR(cp, c, "test_list", vs, "first", 0);
    CHECK_STR(cp, c, "test_list", vs, "second", 1);
    CHECK_STR(cp, c, "test_list", vs, "third", 2);
    CHECK_INT(cp, c, "test_bool", vi, 1, 0);
    CHECK_INT(cp, c, "test_list_inline", vi, 1, 0);
    CHECK_INT(cp, c, "test_list_inline", vi, 2, 1);
    CHECK_INT(cp, c, "test_list_inline", vi, 3, 2);
  }
  config_destroy(conf2);
  return;
}
END_TEST

START_TEST(test_yaml_empty)
{
  config_opt_add(conf, "core", "address", PARAM_STRING, NULL, NULL);
  ck_assert_call_fail(config_parse_string, conf, yaml_no_stream);
  ck_assert_call_fail(config_parse_string, conf, yaml_empty_document);
  ck_assert_call_fail(config_parse_string, conf, yaml_document_no_mapping);
  ck_assert_call_fail(config_parse_string, conf, yaml_no_start_document);
  ck_assert_call_fail(config_parse_string, conf, yaml_no_section);
  ck_assert_call_fail(config_parse_string, conf, yaml_no_section2);
  ck_assert_call_fail(config_parse_string, conf, yaml_no_section_name);
  ck_assert_call_fail(config_parse_string, conf, yaml_no_section_name2);
  ck_assert_call_fail(config_parse_string, conf, yaml_malformed);
}
END_TEST

START_TEST(test_yaml_invalid_data)
{
  config_opt_add(conf, "core", "intvalue", PARAM_INT, NULL, NULL);
  config_opt_add(conf, "core", "boolvalue", PARAM_BOOL, NULL, NULL);
  config_opt_add(conf, "core", "strvalue", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, "core", "singlevalue", PARAM_INT, NULL, NULL);
  ck_assert_call_fail(config_parse_string, conf, NULL);
  ck_assert_call_fail(config_parse_string, NULL, "");
  ck_assert_call_fail(config_parse_string, conf, yaml_wrong_int);
  ck_assert_call_fail(config_parse_string, conf, yaml_wrong_int2);
  ck_assert_call_fail(config_parse_string, conf, yaml_wrong_bool);
  ck_assert_call_fail(config_parse_string, conf, yaml_wrong_string);
  ck_assert_call_fail(config_parse_string, conf, yaml_wrong_multivalued);
}
END_TEST

START_TEST(test_yaml_file)
{
  char *tmp = tmpnam(0);
  FILE *fh = fopen(tmp, "w");
  ck_assert_call_fail(config_parse, conf, "/path/to/config/that/should/not/exists");

  fputs(yaml_good, fh);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "address", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "port", PARAM_INT, NULL, NULL);
  config_opt_set_range_int(conf, RAPP_CONFIG_SECTION, "port", 1, 65535);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "test_list", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "test_list_inline", PARAM_INT, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "test_bool", PARAM_BOOL, NULL, NULL);
  config_opt_set_multivalued(conf, RAPP_CONFIG_SECTION, "test_list", 1);
  config_opt_set_multivalued(conf, RAPP_CONFIG_SECTION, "test_list_inline", 1);
  fclose(fh);
  ck_assert_call_ok(config_parse, conf, tmp);

  chmod(tmp, 0);
  ck_assert_call_fail(config_parse, conf, tmp);
  unlink(tmp);
}
END_TEST

START_TEST(test_yaml_dir)
{
  char tmpl[] = "rapptestXXXXXX";
  char *dir = mkdtemp(tmpl);
  char yaml_path[PATH_MAX];
  char txt_path[PATH_MAX];
  FILE *yh, *th;
  sprintf(yaml_path, "%s/good.yaml", dir);
  sprintf(txt_path, "%s/bad.txt", dir);
  th = fopen(txt_path, "w");
  yh = fopen(yaml_path, "w");
  fputs(yaml_malformed, th);
  fputs(yaml_good, yh);
  fclose(yh);
  fclose(th);


  config_opt_add(conf, RAPP_CONFIG_SECTION, "address", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "port", PARAM_INT, NULL, NULL);
  config_opt_set_range_int(conf, RAPP_CONFIG_SECTION, "port", 1, 65535);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "test_list", PARAM_STRING, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "test_list_inline", PARAM_INT, NULL, NULL);
  config_opt_add(conf, RAPP_CONFIG_SECTION, "test_bool", PARAM_BOOL, NULL, NULL);
  config_opt_set_multivalued(conf, RAPP_CONFIG_SECTION, "test_list", 1);
  config_opt_set_multivalued(conf, RAPP_CONFIG_SECTION, "test_list_inline", 1);

  ck_assert_call_fail(config_scan_directory, conf, dir, ".txt");
  ck_assert_call_ok(config_scan_directory, conf, dir, ".yaml");
  ck_assert_call_ok(config_scan_directory, conf, dir, 0);
  ck_assert_call_fail(config_scan_directory, 0, dir, ".yaml");
  ck_assert_call_fail(config_scan_directory, conf, 0, ".yaml");
  chmod(dir, 0);
  ck_assert_call_fail(config_scan_directory, conf, dir, 0);
  chmod(dir, S_IRUSR | S_IWUSR | S_IXUSR);

  unlink(txt_path);
  unlink(yaml_path);
  rmdir(dir);
}
END_TEST

static Suite *
config_yaml_parse_suite(void)
{
  Suite *s = suite_create("rapp.core.config_yaml");
  TCase *tc = tcase_create("rapp.core.config_yaml");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_yaml_parse);
  tcase_add_test(tc, test_yaml_empty);
  tcase_add_test(tc, test_yaml_invalid_data);
  tcase_add_test(tc, test_yaml_file);
  tcase_add_test(tc, test_yaml_dir);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
  int number_failed = 0;

  Suite *s = config_yaml_parse_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_log(sr, "check_config_yaml.log");
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
