/*
 * check_version.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

/*
 * RApp test version. Use this as template for new tests.
 * Do not forget to add the new test to tests/CMakeLists.txt!
 */

#include <stdlib.h>

#include <check.h>

#include "rapp/rapp_version.h"



START_TEST(test_version_full)
{
  const char *s = rapp_get_version_full();
  ck_assert(s != NULL);
}
END_TEST

START_TEST(test_version)
{
  const char *s = rapp_get_version();
  ck_assert(s != NULL);
}
END_TEST

START_TEST(test_version_sha1)
{
  const char *s = rapp_get_version_sha1();
  ck_assert(s != NULL);
}
END_TEST

START_TEST(test_version_tag)
{
  const char *s = rapp_get_version_tag();
  ck_assert(s != NULL);
}
END_TEST

START_TEST(test_banner)
{
  const char *s = rapp_get_banner();
  ck_assert(s != NULL);
}
END_TEST


static Suite *
version_suite(void)
{
  Suite *s = suite_create("rapp.core.version");
  TCase *tc = tcase_create("rapp.core.version");

  tcase_add_test(tc, test_version_full);
  tcase_add_test(tc, test_version);
  tcase_add_test(tc, test_version_sha1);
  tcase_add_test(tc, test_version_tag);
  tcase_add_test(tc, test_banner);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = version_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

