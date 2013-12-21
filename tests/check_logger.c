/*
 * check_logger.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <check.h>

#include <logger.h>


START_TEST(test_logger_null)
{
  int err = 0;
  struct Logger *logger = logger_new_null();
  ck_assert(logger != NULL);
  err = logger_trace(logger, LOG_CRITICAL, __FILE__, "this is going to be discarded!");
  ck_assert_int_eq(err, 0);
  err = logger_flush(logger);
  ck_assert_int_eq(err, 0);
  logger_destroy(logger);
}
END_TEST


static void
ck_assert_file_content(FILE *fp, const char *content, size_t len)
{
  int got = 0;
  char *buf = 0;
  ck_assert(len >= 0);

  buf = calloc(1, len+1);
  ck_assert(buf != NULL);

  rewind(fp);
  got = fread(buf, 1, len, fp);
  ck_assert_int_eq(got, len);
  ck_assert_str_eq(buf, content);

  free(buf);
}

#define PANIC_MSG "[CRI] panic!\n"

START_TEST(test_logger_panic)
{
  int err = 0;
  FILE *stderr_save = stderr;

  stderr = tmpfile();
  ck_assert(stderr != NULL);

  logger_panic("panic!");

  ck_assert_file_content(stderr, PANIC_MSG, strlen(PANIC_MSG));

  fclose(stderr);
  stderr = stderr_save;
}
END_TEST


#define TEST_MSG  "test message"

START_TEST(test_logger_file)
{
  int err = 0;
  FILE *sink = NULL;
  struct Logger *logger = NULL;

  sink = tmpfile();
  ck_assert(sink != NULL);

  logger = logger_new_file(LOG_MARK, sink);
  ck_assert(logger != NULL);
  err = logger_trace(logger, LOG_CRITICAL, __FILE__, TEST_MSG);
  ck_assert_int_eq(err, 0);
  err = logger_flush(logger);
  ck_assert_int_eq(err, 0);
  logger_destroy(logger);

  ck_assert_file_content(sink, TEST_MSG, strlen(TEST_MSG));

  fclose(sink);
}
END_TEST


#define HUGE_BUF_SZ   2048
#define HUGE_MSG_LEN  2000

START_TEST(test_logger_file_huge_msg)
{
  char huge[HUGE_BUF_SZ] = { '\0' };
  int err = 0;
  int i = 0;
  FILE *sink = NULL;
  FILE *stderr_save = stderr;
  struct Logger *logger = NULL;

  ck_assert(HUGE_BUF_SZ > HUGE_MSG_LEN);

  /* FIXME: ugly */
  for (i = 0; i < HUGE_MSG_LEN; i++) {
    huge[i] = 'Z';
  }

  stderr = tmpfile();
  ck_assert(stderr != NULL);

  sink = tmpfile();
  ck_assert(sink != NULL);

  logger = logger_new_file(LOG_MARK, sink);
  ck_assert(logger != NULL);
  err = logger_trace(logger, LOG_CRITICAL, __FILE__, huge);
  /*
   * careful here. The format string must be huge,
   * not necessarily the message being logged.
   */
  ck_assert_int_eq(err, 0);
  err = logger_flush(logger);
  ck_assert_int_eq(err, 0);
  logger_destroy(logger);

  ck_assert_file_content(sink, huge, strlen(huge));
  fclose(sink);

  ck_assert_file_content(stderr, "", 0);
  /* nothing on stderr */
  fclose(stderr);

  stderr = stderr_save;
}
END_TEST


START_TEST(test_logger_console_plain)
{
  int err = 0;
  FILE *sink = NULL;
  struct Logger *logger = NULL;

  sink = tmpfile();
  ck_assert(sink != NULL);

  logger = logger_new_console(LOG_MARK, sink);
  ck_assert(logger != NULL);
  err = logger_trace(logger, LOG_MARK, __FILE__, TEST_MSG);
  ck_assert_int_eq(err, 0);
  err = logger_flush(logger);
  ck_assert_int_eq(err, 0);
  logger_destroy(logger);

  ck_assert_file_content(sink, TEST_MSG, strlen(TEST_MSG));
}
END_TEST

/* FIXME: handle multiple writes. */
struct Buffer {
  char *data;
  size_t size;
};

static int
buffer_logger(void       *data,
              LogLevel    level,
              const char *tag,
              const char *fmt,
              va_list     args)
{
  struct Buffer *buf = data;

  ck_assert(buf != NULL);
  ck_assert(buf->data != NULL);
  ck_assert(buf->size > 0);

  vsnprintf(buf->data, buf->size, fmt, args);
  return 0;
}


START_TEST(test_logger_custom)
{
  int err = 0;
  char buf[128] = { '\0' }; /* FIXME: hardcoded size */
  struct Buffer logbuf;
  struct Logger *logger = NULL;

  logbuf.data = buf;
  logbuf.size = sizeof(buf);

  logger = logger_new_custom(LOG_MARK, buffer_logger, &logbuf);
  ck_assert(logger != NULL);
  err = logger_trace(logger, LOG_MARK, "", TEST_MSG);
  ck_assert_int_eq(err, 0);
  err = logger_flush(logger);
  ck_assert_int_eq(err, 0);
  logger_destroy(logger);

  ck_assert_str_eq(logbuf.data, TEST_MSG);
}
END_TEST

static Suite *
logger_suite(void)
{
  Suite *s = suite_create("rapp.core.logger");
  TCase *tc = tcase_create("rapp.core.logger");

  tcase_add_test(tc, test_logger_panic);
  tcase_add_test(tc, test_logger_null);
  tcase_add_test(tc, test_logger_file);
  tcase_add_test(tc, test_logger_file_huge_msg);
  tcase_add_test(tc, test_logger_console_plain);
  tcase_add_test(tc, test_logger_custom);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = logger_suite();
 SRunner *sr = srunner_create(s);

 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

