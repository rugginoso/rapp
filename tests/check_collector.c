#include <stdlib.h>
#include <check.h>

#include <collector.h>

static char *message = "collect me";

static void
free_func(void *data)
{
  ck_assert_str_eq(data, (const char *)message);
}


START_TEST(test_collector_calls_free_func_on_collect)
{
  struct Collector *collector = collector_new();

  collector_schedule_free(collector, free_func, message);
  collector_collect(collector);

  collector_destroy(collector);
}
END_TEST


static Suite *
collector_suite(void)
{
  Suite *s = suite_create("rapp.core.collector");
  TCase *tc = tcase_create("rapp.core.collector");

  tcase_add_test(tc, test_collector_calls_free_func_on_collect);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = collector_suite ();
 SRunner *sr = srunner_create (s);

 srunner_run_all (sr, CK_NORMAL);
 number_failed = srunner_ntests_failed (sr);
 srunner_free (sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
