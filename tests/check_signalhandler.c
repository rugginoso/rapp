#include <stdlib.h>
#include <check.h>

#include <signal.h>

#include <eloop.h>
#include <signalhandler.h>

struct ELoop *eloop = NULL;
struct SignalHandler *signal_handler = NULL;

static void
on_signal(struct SignalHandler *signal_handler, void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  event_loop_stop(eloop);
}

void
setup(void)
{
  eloop = event_loop_new();
  signal_handler = signal_handler_new(eloop);
}

void teardown(void)
{
  signal_handler_destroy(signal_handler);
  event_loop_destroy(eloop);
}

START_TEST(test_signal_handler_calls_callback_when_signal_arrives)
{
  signal_handler_add_signal_callback(signal_handler, SIGUSR1, on_signal, eloop);

  kill(getpid(), SIGUSR1);

  event_loop_run(eloop);
}
END_TEST


static Suite *
signal_handler_suite(void)
{
  Suite *s = suite_create("rapp.core.eloop");
  TCase *tc = tcase_create("rapp.core.eloop");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_signal_handler_calls_callback_when_signal_arrives);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = signal_handler_suite ();
 SRunner *sr = srunner_create (s);

 srunner_run_all (sr, CK_NORMAL);
 number_failed = srunner_ntests_failed (sr);
 srunner_free (sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
