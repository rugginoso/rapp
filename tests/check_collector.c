/*
 * check_collector.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <check.h>

#include <logger.h>
#include <collector.h>

#include "test_utils.h"


#define MESSAGE "collect me"
#define MESSAGE_LEN STRLEN(MESSAGE)

char buf[MESSAGE_LEN];
struct Collector *collector = NULL;
int calls = 0;
struct Logger *logger;

static void
free_func(void *data)
{
  calls++;
  memcpy(buf, MESSAGE, MESSAGE_LEN);
}

void setup()
{
  logger = logger_new_null();
  collector = collector_new(logger);
  memset(buf, 0, MESSAGE_LEN);
  calls = 0;
}

void teardown()
{
  collector_destroy(collector);
  logger_destroy(logger);
}

START_TEST(test_collector_calls_free_func_on_collect)
{
  collector_schedule_free(collector, free_func, MESSAGE);

  collector_collect(collector);

  ck_assert_str_eq(buf, MESSAGE);

}
END_TEST

START_TEST(test_collector_calls_free_func_only_one_time_per_object)
{
  collector_schedule_free(collector, free_func, MESSAGE);
  collector_schedule_free(collector, free_func, MESSAGE);

  collector_collect(collector);

  ck_assert_str_eq(buf, MESSAGE);
  ck_assert_int_eq(calls, 1);
}
END_TEST


static Suite *
collector_suite(void)
{
  Suite *s = suite_create("rapp.core.collector");
  TCase *tc = tcase_create("rapp.core.collector");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_collector_calls_free_func_on_collect);
  tcase_add_test(tc, test_collector_calls_free_func_only_one_time_per_object);
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
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

