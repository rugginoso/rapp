#include <stdlib.h>
#include <check.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <eloop.h>

static const char *message = "hello world";

static int
read_func(int         fd,
          const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;
  char buf[strlen(message) + 1];

  read(fd, buf, strlen(message));
  buf[strlen(message)] = 0;

  ck_assert_str_eq(buf, message);

  event_loop_stop(eloop);

  return 0;
}

static int
write_func(int         fd,
           const void *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  write(fd, message, strlen(message) + 1);

  event_loop_stop(eloop);

  return 0;
}


START_TEST(test_eloop_calls_read_func_on_pending_data)
{
  struct ELoop *eloop = event_loop_new();
  mkfifo("check_eloop.fifo", 0600);
  int fd = open("check_eloop.fifo", O_RDWR);

  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
  callbacks[ELOOP_CALLBACK_READ] = read_func;
  callbacks[ELOOP_CALLBACK_WRITE] = NULL;
  callbacks[ELOOP_CALLBACK_CLOSE] = NULL;

  event_loop_add_fd_watch(eloop, fd, callbacks, eloop);

  write(fd, message, strlen(message));

  event_loop_run(eloop);

  close(fd);
  unlink("check_eloop.fifo");
  event_loop_destroy(eloop);
}
END_TEST


START_TEST(test_eloop_calls_write_func_when_fd_becomes_writable)
{
  struct ELoop *eloop = event_loop_new();
  mkfifo("check_eloop.fifo", 0600);
  int fd = open("check_eloop.fifo", O_RDWR);
  char buf[strlen(message) + 1];

  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
  callbacks[ELOOP_CALLBACK_READ] = NULL;
  callbacks[ELOOP_CALLBACK_WRITE] = write_func;
  callbacks[ELOOP_CALLBACK_CLOSE] = NULL;

  event_loop_add_fd_watch(eloop, fd, callbacks, eloop);

  event_loop_run(eloop);

  read(fd, buf, strlen(message));
  buf[strlen(message)] = 0;

  ck_assert_str_eq(buf, message);

  close(fd);
  unlink("check_eloop.fifo");
  event_loop_destroy(eloop);
}
END_TEST


static Suite *
eloop_suite(void)
{
  Suite *s = suite_create("rapp.core.eloop");
  TCase *tc = tcase_create("rapp.core.eloop");

  tcase_add_test(tc, test_eloop_calls_read_func_on_pending_data);
  tcase_add_test(tc, test_eloop_calls_write_func_when_fd_becomes_writable);
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
