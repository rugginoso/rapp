/*
 * eloop.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef ELOOP_H
#define ELOOP_H

#include "collector.h"

struct ELoop;

typedef int (*ELoopWatchFdCallback)(int fd, const void *data);

enum ELoopWatchFdCallbackType {
  ELOOP_CALLBACK_READ = 0,
  ELOOP_CALLBACK_WRITE,
  ELOOP_CALLBACK_CLOSE,
  ELOOP_CALLBACK_MAX,
};

struct ELoop *event_loop_new(void);
void event_loop_destroy(struct ELoop *eloop);

int event_loop_run(struct ELoop *eloop);
void event_loop_stop(struct ELoop *eloop);

int event_loop_add_fd_watch(struct ELoop *eloop, int fd, enum ELoopWatchFdCallbackType callback_type, ELoopWatchFdCallback callback, const void *data);

int event_loop_remove_fd_watch(struct ELoop *eloop, int fd, enum ELoopWatchFdCallbackType callback_type);

void event_loop_schedule_free(struct ELoop *eloop, CollectorFreeFunc free_func, void *data);

#endif /* ELOOP_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

