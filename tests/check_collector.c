#include <stdlib.h>
#include <check.h>

#include <collector.h>

#include "test_utils.h"


#define MESSAGE "collect me"
#define MESSAGE_LEN STRLEN(MESSAGE)

char buf[MESSAGE_LEN];
struct Collector *collector = NULL;

static void
free_func(void *data)
{
  memcpy(buf, MESSAGE, MESSAGE_LEN);
}

void setup()
{
  collector = collector_new();
  memset(buf, 0, MESSAGE_LEN);
}

void teardown()
{
  collector_destroy(collector);
}

START_TEST(test_collector_calls_free_func_on_collect)
{
  collector_schedule_free(collector, free_func, MESSAGE);

  collector_collect(collector);

  ck_assert_str_eq(buf, MESSAGE);

}
END_TEST


static Suite *
collector_suite(void)
{
  Suite *s = suite_create("rapp.core.collector");
  TCase *tc = tcase_create("rapp.core.collector");

  tcase_add_checked_fixture (tc, setup, teardown);
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
