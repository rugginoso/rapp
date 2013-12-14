#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config_private.h>
#include <config_api.h>
#include <logger.h>

#include "test_utils.h"

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
const char *yaml_good =
"---\n"
"core:\n"
"    address : \"127.0.0.1\"\n"
"    port: 8080 \n"
"    test_list:\n"
"        - first\n"
"        - second\n"
"        - third\n"
"    test_bool: y\n"
"    test_list_inline: [1,2,3]\n"
"skipped:\n"
"    this: \"is skipped\"\n"
"    with_all: \"options\"\n";

const char *yaml_good_inline =
"---\n"
"core: {address: \"127.0.0.2\", port: 8081, test_list: [\"first1\", \"second1\", \"third1\"], test_bool: no, test_list_inline: [4,5,6]}";

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

START_TEST(test_config_yaml_parse)
{
    long vi;
    char *vs;
    const char *c = "core";

    config_opt_add(conf, c, "address", PARAM_STRING, NULL);
    config_opt_add(conf, c, "port", PARAM_INT, NULL);
    config_opt_set_range_int(conf, c, "port", 1, 65535);
    config_opt_add(conf, c, "test_list", PARAM_STRING, NULL);
    config_opt_add(conf, c, "test_list_inline", PARAM_INT, NULL);
    config_opt_add(conf, c, "test_bool", PARAM_BOOL, NULL);
    config_opt_set_multivalued(conf, c, "test_list", 1);
    config_opt_set_multivalued(conf, c, "test_list_inline", 1);

    config_parse_string(conf, yaml_good);
    CHECK_STR(conf, c, "address", vs, "127.0.0.1", 0);
    CHECK_INT(conf, c, "port", vi, 8080, 0);
    CHECK_STR(conf, c, "test_list", vs, "first", 0);
    CHECK_STR(conf, c, "test_list", vs, "second", 1);
    CHECK_STR(conf, c, "test_list", vs, "third", 2);
    CHECK_INT(conf, c, "test_bool", vi, 1, 0);
    CHECK_INT(conf, c, "test_list_inline", vi, 1, 0);
    CHECK_INT(conf, c, "test_list_inline", vi, 2, 1);
    CHECK_INT(conf, c, "test_list_inline", vi, 3, 2);

    config_parse_string(conf, yaml_good_inline);

    return;
}
END_TEST


static Suite *
config_yaml_parse_suite(void)
{
  Suite *s = suite_create("rapp.core.config.yaml");
  TCase *tc = tcase_create("rapp.core.config.yaml");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_yaml_parse);
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

