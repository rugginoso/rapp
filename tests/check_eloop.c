#include <stdlib.h>
#include <check.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <eloop.h>

#define FIFO_NAME "check_eloop.fifo"
#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

static const char *message = "hello world";

struct ELoop *eloop = NULL;
int watched_fd = -1;
ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
char buf[STRLEN(message)];

static int
read_func(int         fd,
          const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  ck_assert_int_eq(fd, watched_fd);

  read(fd, buf, strlen(message));

  event_loop_stop(eloop);

  return 0;
}

static int
write_func(int         fd,
           const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  ck_assert_int_eq(fd, watched_fd);

  write(fd, message, strlen(message) + 1);

  event_loop_stop(eloop);

  return 0;
}

static int
close_func(int         fd,
           const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  event_loop_stop(eloop);

  return 0;
}

void
setup(void)
{
  eloop = event_loop_new();

  mkfifo(FIFO_NAME, 0600);
  watched_fd = open("check_eloop.fifo", O_RDWR);

  memset(callbacks, 0, sizeof(ELoopWatchFdCallback) * ELOOP_CALLBACK_MAX);
  memset(buf, 0, strlen(message) + 1);
}

void teardown(void)
{
  close(watched_fd);
  unlink(FIFO_NAME);
  event_loop_destroy(eloop);
}

START_TEST(test_eloop_calls_read_func_when_fd_has_pending_data)
{
  callbacks[ELOOP_CALLBACK_READ] = read_func;

  event_loop_add_fd_watch(eloop, watched_fd, callbacks, eloop);

  write(watched_fd, message, strlen(message));

  event_loop_run(eloop);

  ck_assert_str_eq(buf, message);
}
END_TEST


START_TEST(test_eloop_calls_write_func_when_fd_becomes_writable)
{
  callbacks[ELOOP_CALLBACK_WRITE] = write_func;

  event_loop_add_fd_watch(eloop, watched_fd, callbacks, eloop);

  event_loop_run(eloop);

  read(watched_fd, buf, strlen(message));
  buf[strlen(message)] = 0;

  ck_assert_str_eq(buf, message);
}
END_TEST


START_TEST(test_eloop_calls_close_func_when_fd_is_closed)
{
  callbacks[ELOOP_CALLBACK_CLOSE] = close_func;

  event_loop_add_fd_watch(eloop, watched_fd, callbacks, eloop);

  close(watched_fd);

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
