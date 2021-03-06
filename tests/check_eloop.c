/*
 * check_eloop.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <check.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <logger.h>
#include <eloop.h>

#include "test_memstubs.h"
#include "test_utils.h"

#define MESSAGE "Hello world!"
#define MESSAGE_LEN STRLEN(MESSAGE)

#define WATCHED 0
#define OTHER 1

struct ELoop *eloop = NULL;
ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
char buf[MESSAGE_LEN];
int fds[2];
struct Logger *logger;

static int
read_func(int         fd,
          const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  ck_assert_int_eq(fd, fds[WATCHED]);

  read(fd, buf, MESSAGE_LEN);

  event_loop_stop(eloop);

  return 0;
}

static int
write_func(int         fd,
           const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  ck_assert_int_eq(fd, fds[WATCHED]);

  write(fd, MESSAGE, MESSAGE_LEN);

  event_loop_stop(eloop);

  return 0;
}

static int
close_func(int         fd,
           const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  ck_assert_int_eq(fd, fds[WATCHED]);

  event_loop_stop(eloop);

  return 0;
}

static void
free_func(void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  event_loop_stop(eloop);
}

void
setup(void)
{
  logger = logger_new_null();
  eloop = event_loop_new(logger);

  socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

  memset(callbacks, 0, sizeof(ELoopWatchFdCallback) * ELOOP_CALLBACK_MAX);
  memset(buf, 0, MESSAGE_LEN);
}

void teardown(void)
{
  close(fds[WATCHED]);
  close(fds[OTHER]);
  event_loop_destroy(eloop);
  logger_destroy(logger);
}

START_TEST(test_eloop_calls_read_func_when_fd_has_pending_data)
{
  event_loop_add_fd_watch(eloop, fds[WATCHED], ELOOP_CALLBACK_READ, read_func, eloop);

  write(fds[OTHER], MESSAGE, MESSAGE_LEN);

  event_loop_run(eloop);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST


START_TEST(test_eloop_calls_write_func_when_fd_becomes_writable)
{
  event_loop_add_fd_watch(eloop, fds[WATCHED], ELOOP_CALLBACK_WRITE, write_func, eloop);

  event_loop_run(eloop);

  read(fds[OTHER], buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST


START_TEST(test_eloop_calls_close_func_when_fd_is_closed)
{
  event_loop_add_fd_watch(eloop, fds[WATCHED], ELOOP_CALLBACK_CLOSE, close_func, eloop);

  close(fds[OTHER]);

  event_loop_run(eloop);
}
END_TEST

START_TEST(test_eloop_calls_free_func_when_is_scheduled)
{
  event_loop_schedule_free(eloop, free_func, eloop);
}
END_TEST

START_TEST(test_eloop_new_fails)
{
  struct Logger *logger = logger_new_null();
  struct ELoop *eloop = NULL;
  memstub_failure_enable(0, 1);
  eloop = event_loop_new(logger);
  ck_assert(eloop == NULL);
}
END_TEST

static Suite *
eloop_suite(void)
{
  Suite *s = suite_create("rapp.core.eloop");
  TCase *tc = tcase_create("rapp.core.eloop");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_eloop_calls_read_func_when_fd_has_pending_data);
  tcase_add_test(tc, test_eloop_calls_write_func_when_fd_becomes_writable);
  tcase_add_test(tc, test_eloop_calls_close_func_when_fd_is_closed);
  tcase_add_test(tc, test_eloop_calls_free_func_when_is_scheduled);
  tcase_add_test(tc, test_eloop_new_fails);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = eloop_suite ();
 SRunner *sr = srunner_create (s);

 srunner_run_all (sr, CK_NORMAL);
 number_failed = srunner_ntests_failed (sr);
 srunner_free (sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

