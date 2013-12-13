#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config_private.h>
#include <config_api.h>
#include <logger.h>

#include "test_utils.h"

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


START_TEST(test_config_yaml_parse)
{
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

