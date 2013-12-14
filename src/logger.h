#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include "rapp/rapp_logger.h"


typedef int (*LogHandlerCallback)(void *data, LogLevel level, const char *tag, const char *fmt, va_list args);


struct Logger *logger_open_null(void);
struct Logger *logger_open_file(LogLevel max_level, FILE *sink);
struct Logger *logger_open_console(LogLevel max_level, FILE *sink);
struct Logger *logger_open_custom(LogLevel max_level, LogHandlerCallback log_handler, void *user_data);

void logger_destroy(struct Logger *logger);

int logger_flush(struct Logger *logger);

int logger_panic(const char *msg);

#endif  /* LOGGER_H */

