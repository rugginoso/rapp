#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "logger.h"


struct Logger {
  void *priv;

  LogLevel max_level;

  LogHandlerCallback trace;
  int (*close)(struct Logger *logger);
  int (*flush)(struct Logger *logger);
};

#define MAX(a, b)   (((a) > (b)) ?(a) :(b))
#define MIN(a, b)   (((a) < (b)) ?(a) :(b))
/* clamp x between a and b */
#define CLAMP(x, a, b)  MIN(MAX((a), (x)), (b))



/* colors macros */
#define COL(x)              "\033[" #x ";1m"
#define COL_RED             COL(31)
#define COL_GREEN           COL(32)
#define COL_YELLOW          COL(33)
#define COL_BLUE            COL(34)
#define COL_WHITE           COL(37)
#define COL_GRAY            "\033[0m"

enum {
  LOG_BUF_SIZE     = 1024,
  LOG_TEMPLATE_LEN = 32 /* upper bound, really */
};

static const char *
logger_template(LogLevel level)
{
  /* WARNING: we MUST keep in sync templates order with LOG* macros */
  static const char *logger_templates[] = {
    COL_RED"CRI [%s]: %s"COL_GRAY"\n",    /* LOG_CRITICAL */
    COL_RED"ERR [%s]"COL_GRAY": %s\n",    /* LOG_ERROROR    */
    COL_YELLOW"WRN [%s]"COL_GRAY": %s\n", /* LOG_WARNING  */
    COL_GREEN"INF [%s]"COL_GRAY": %s\n",  /* LOG_INFO     */
    COL_BLUE"DBG [%s]"COL_GRAY": %s\n",   /* LOG_DEBUG    */
    "%s%s" /* LOG_MARK: the tag placeholder must be present but tag
              value will be ignored */
    "%s%s" /* LOG_LAST: only for safety */
  };
  return logger_templates[level - LOG_CRITICAL];
}

static int
logger_trace_console(void       *user_data,
                     LogLevel    level,
                     const char *tag,
                     const char *fmt,
                     va_list     ap)
{
  int ret = 0;
  int is_dynbuf = 0;
  /* flag: we must use a dynamic (larger than static) buffer? */
  char buf[LOG_BUF_SIZE];
  char *msg = buf;
  size_t size = sizeof(buf);
  const char *template = NULL;

  assert(tag != NULL);
  assert(fmt != NULL);

  tag = (tag != NULL) ?tag :"";
  /* LOG_EXTRA special handling: force always empty tag */
  tag = (level != LOG_MARK) ?tag :"";
  fmt = (fmt != NULL) ?fmt :"";
  template = logger_template(level);

  size = LOG_TEMPLATE_LEN + strlen(tag) + strlen(fmt) + 1;

  if (size > sizeof(buf)) {
    /*
     * we use malloc/fprintf instead of malloc because
     * we want custom error messages
     */
    msg = malloc(size);
    if (msg != NULL) {
      is_dynbuf = 1;
    } else {
      fprintf(stderr, "(%s) CRITICAL: can't get memory in "
              "logger_trace(), the output will be truncated.\n",
               __FILE__);
       /* force reset to default values */
       msg = buf;
       size = sizeof(buf) - 1;
       ret = 1;
    }
  } else {
    size = sizeof(buf) - 1; // FIXME
  }

  /* construct real format string */
  snprintf(msg, size, template, tag, fmt);
  /* and then finally deliver the message */
  vfprintf(user_data, msg, ap);

  if (is_dynbuf) {
    free(msg);
  }
  return ret;
}

/* for extreme circumstances, like unable to creae a logger. */
int
logger_panic(const char *msg)
{
  fprintf(stderr, "[CRI] %s\n", msg);
  return 0;
}

static int
logger_trace_file(void       *user_data,
                  LogLevel    level,
                  const char *tag,
                  const char *fmt,
                  va_list     ap)
{
  /* FIXME: needs formatting */
  return logger_trace_console(user_data, LOG_MARK, tag, fmt, ap);
}

static int
logger_flush_file(struct Logger *logger)
{
  int err = fflush(logger->priv);
  if (err == EOF) {
    err = -1;
  }
  return err;
}

static int
logger_destroy_file(struct Logger *logger)
{
  /* the file ownership isn't yours, so we just
     want to make sure we delivered everything
  */
  return logger_flush_file(logger);
}

static int
logger_trace_null(void       *user_data,
                  LogLevel    level,
                  const char *tag,
                  const char *fmt,
                  va_list     ap)
{
  return 0;
}

static int
logger_destroy_null(struct Logger *logger)
{
  return 0;
}

static int
logger_flush_null(struct Logger *logger)
{
  return 0;
}

struct Logger *
logger_open_null(void)
{
  struct Logger *logger = calloc(1, sizeof(struct Logger));
  if (logger) {
    logger->priv = NULL;
    logger->max_level = -1;
    logger->trace = logger_trace_null;
    logger->close = logger_destroy_null;
    logger->flush = logger_flush_null;
  }
  return logger;
}

struct Logger *
logger_open_file(LogLevel max_level,
                 FILE    *sink)
{
  LogLevel lev = CLAMP(max_level, LOG_ERROR, LOG_MARK); /* TODO */
  struct Logger *logger = NULL;

  if (sink) {
    logger = calloc(1, sizeof(struct Logger));
  }
  if (logger) {
    logger->priv = sink;
    logger->max_level = lev;
    logger->trace = logger_trace_file;
    logger->close = logger_destroy_file;
    logger->close = logger_flush_file;
  }
  return logger;
}


struct Logger *
logger_open_console(LogLevel max_level,
                    FILE    *sink)
{
  FILE *con = (sink) ?sink :stderr;
  LogLevel lev = CLAMP(max_level, LOG_ERROR, LOG_MARK); /* TODO */

  struct Logger *logger = calloc(1, sizeof(struct Logger));
  if (logger) {
    logger->priv = con;
    logger->max_level = lev;
    logger->trace = logger_trace_console;
    logger->close = logger_destroy_null;
    logger->close = logger_flush_file;
  }
  return logger;
}

struct Logger *
logger_open_custom(LogLevel           max_level,
                   LogHandlerCallback logger_handler,
                   void              *user_data)
{
  LogLevel lev = CLAMP(max_level, LOG_ERROR, LOG_MARK); /* TODO */

  struct Logger *logger = calloc(1, sizeof(struct Logger));
  if (logger) {
    logger->priv = user_data;
    logger->max_level = lev;
    logger->trace = logger_handler;
    logger->close = logger_destroy_null;
    logger->close = logger_flush_null;
  }
  return logger;
}

int
logger_trace(struct Logger *logger,
             LogLevel       level,
             const char    *tag,
             const char    *fmt,
             ...)
{
  int err = 0;
  va_list args;

  assert(logger);
  assert(tag);
  assert(fmt);

  va_start(args, fmt);
  err = logger_trace_va(logger, level, tag, fmt, args);
  va_end(args);

  return err;
}

int
logger_trace_va(struct Logger *logger,
                LogLevel       level,
                const char    *tag,
                const char    *fmt,
                va_list        args)
{
  int err = 0;

  assert(logger);
  assert(tag);
  assert(fmt);

  level = CLAMP(level, LOG_ERROR, LOG_MARK); /* TODO */

  if (logger->max_level >= level) {
    err = logger->trace(logger->priv, level, tag, fmt, args);
  }
  return err;
}

int
logger_flush(struct Logger *logger)
{
  if (!logger) {
    return -1;
  }
  return logger->flush(logger);
}

void
logger_destroy(struct Logger *logger)
{
  int err = -1;
  if (logger) {
     err = logger->close(logger);
     if (!err) {
        free(logger);
     }
  }
}

/* vim: set sw=2 ts=2 et */

