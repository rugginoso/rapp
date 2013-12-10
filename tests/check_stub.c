/*
 * RApp test stub. Use this as template for new tests.
 * Do not forget to add the new test to tests/CMakeLists.txt!
 */

#include <stdlib.h>

#include <check.h>

/* include the header(s) of the tested code right after */



START_TEST(test_stub_example)
{
  return;/* fill me with a meaningful test! */
}
END_TEST


static Suite *
stub_suite(void)
{
  Suite *s = suite_create("rapp.core.stub");
  TCase *tc = tcase_create("rapp.core.stub");

  tcase_add_test(tc, test_stub_example);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = stub_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

