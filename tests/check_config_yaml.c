#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config_private.h>
#include <config_api.h>
#include <logger.h>

#include "test_utils.h"
#include "yaml_documents.h"

#define CHECK_STR(CONF, SECT, NAME, STR, EXPECTED, POS)         \
do {                                                            \
    config_get_nth_string(CONF, SECT, NAME, POS, &STR);         \
    ck_assert_str_eq(STR, EXPECTED);                            \
    free(STR);                                                  \
} while(0)

#define CHECK_INT(CONF, SECT, NAME, VAL, EXPECTED, POS)         \
do {                                                            \
    config_get_nth_int(CONF, SECT, NAME, POS, &VAL);            \
    ck_assert_int_eq(VAL, EXPECTED);                            \
} while(0)

struct Config *conf;

void
setup(void)
{
   struct Logger *logger = logger_new_console(LOG_LAST, stderr);
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
    struct Logger *logger = logger_new_console(LOG_LAST, stderr);
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

static Suite *
config_yaml_parse_suite(void)
{
  Suite *s = suite_create("rapp.core.config_yaml");
  TCase *tc = tcase_create("rapp.core.config_yaml");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_yaml_parse);
  tcase_add_test(tc, test_yaml_empty);
  tcase_add_test(tc, test_yaml_invalid_data);
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

