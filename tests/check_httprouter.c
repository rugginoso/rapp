
#include <stdlib.h>

#include <check.h>

#include "logger.h"
#include "httprouter.h"


START_TEST(test_httprouter_new_destroy)
{
  struct Logger *logger = NULL;
  struct HTTPRouter *router = NULL;
  logger = logger_new_null();
  router = http_router_new(logger);
  ck_assert(router != NULL);
  http_router_destroy(router);
  logger_destroy(logger);
}
END_TEST


static Suite *
httprouter_suite(void)
{
  Suite *s = suite_create("rapp.core.httprouter");
  TCase *tc = tcase_create("rapp.core.httprouter");

  tcase_add_test(tc, test_httprouter_new_destroy);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = httprouter_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

