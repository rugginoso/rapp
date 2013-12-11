#include <stdlib.h>
#include <check.h>

#include <sys/socket.h>

#include <eloop.h>
#include "test_utils.h"

#define MESSAGE "Hello world!"
#define MESSAGE_LEN STRLEN(MESSAGE)

#define WATCHED 0
#define OTHER 1

struct ELoop *eloop = NULL;
ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
char buf[MESSAGE_LEN];
int fds[2];

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

void
setup(void)
{
  eloop = event_loop_new();

  socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

  memset(callbacks, 0, sizeof(ELoopWatchFdCallback) * ELOOP_CALLBACK_MAX);
  memset(buf, 0, MESSAGE_LEN);
}

void teardown(void)
{
  close(fds[WATCHED]);
  close(fds[OTHER]);
  event_loop_destroy(eloop);
}

START_TEST(test_eloop_calls_read_func_when_fd_has_pending_data)
{
  callbacks[ELOOP_CALLBACK_READ] = read_func;

  event_loop_add_fd_watch(eloop, fds[WATCHED], callbacks, eloop);

  write(fds[OTHER], MESSAGE, MESSAGE_LEN);

  event_loop_run(eloop);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST


START_TEST(test_eloop_calls_write_func_when_fd_becomes_writable)
{
  callbacks[ELOOP_CALLBACK_WRITE] = write_func;

  event_loop_add_fd_watch(eloop, fds[WATCHED], callbacks, eloop);

  event_loop_run(eloop);

  read(fds[OTHER], buf, MESSAGE_LEN);

  ck_assert_str_eq(buf, MESSAGE);
}
END_TEST


START_TEST(test_eloop_calls_close_func_when_fd_is_closed)
{
  callbacks[ELOOP_CALLBACK_CLOSE] = close_func;

  event_loop_add_fd_watch(eloop, fds[WATCHED], callbacks, eloop);

  close(fds[OTHER]);

  event_loop_run(eloop);
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
