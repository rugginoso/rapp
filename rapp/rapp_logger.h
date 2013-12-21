/*
 * rapp_logger.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_LOGGER_H
#define RAPP_LOGGER_H

#include <stdarg.h>

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


struct Logger *logger_get(void *cookie);

int logger_trace(struct Logger *logger, LogLevel level, const char *tag, const char *fmt, ...);

int logger_trace_va(struct Logger *logger, LogLevel level, const char *tag, const char *fmt, va_list args);

int logger_flush(struct Logger *logger);

#endif /* RAPP_LOGGER_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

