#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config_private.h>


START_TEST(test_config_api)
{
  return;/* fill me with a meaningful test! */
}
END_TEST


static Suite *
config_suite(void)
{
  Suite *s = suite_create("rapp.core.config");
  TCase *tc = tcase_create("rapp.core.config");

  tcase_add_test(tc, test_config_api);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = config_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

