#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdio.h>

struct Logger; 

typedef enum {
    LOG_CRITICAL = 0, /* this MUST be the first and it is
                         the most important. -- PANIC!       */
    LOG_ERROR,        /* you'll need to see this             */
    LOG_WARNING,      /* you'd better to see this            */
    LOG_INFO,         /* informative messages (for tuning)   */
    LOG_DEBUG,        /* debug messages (for devs)           */
    LOG_MARK,         /* verbatim, don't add anything, ever. */
    LOG_LAST          /* this MUST be the last -- 
                         and should'nt be used               */
} LogLevel;


typedef int (*LogHandlerFunc)(void *data,
                              LogLevel level, const char *tag,
                              const char *fmt, va_list args);


struct Logger *logger_open_null(void);

struct Logger *logger_open_file(LogLevel max_level, FILE *sink);

struct Logger *logger_open_console(LogLevel max_level, FILE *sink);

struct Logger *logger_open_custom(LogLevel max_level,
                                  LogHandlerFunc log_handler,
                                  void *user_data);

int logger_close(struct Logger *logger);

int logger_trace(struct Logger *logger,
                 LogLevel level, const char *tag, const char *fmt, ...);

int logger_trace_va(struct Logger *logger,
                    LogLevel level, const char *tag, const char *fmt,
                    va_list args);

int logger_flush(struct Logger *logger);

int logger_panic(const char *msg);

#endif  /* LOGGER_H */

