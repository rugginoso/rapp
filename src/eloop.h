#ifndef ELOOP_H
#define ELOOP_H

#include "collector.h"

struct ELoop;

typedef int (*ELoopWatchFdCallback)(int fd, const void *data);

enum {
  ELOOP_CALLBACK_READ = 0,
  ELOOP_CALLBACK_WRITE,
  ELOOP_CALLBACK_MAX,
};

struct ELoop *event_loop_new(void);
void event_loop_destroy(struct ELoop *eloop);

int event_loop_run(struct ELoop *eloop);
void event_loop_stop(struct ELoop *eloop);

int event_loop_add_fd_watch(struct ELoop *eloop, int fd, ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX], const void *data);
int event_loop_remove_fd_watch(struct ELoop *loop, int fd);

void event_loop_schedule_free(struct ELoop *eloop, CollectorFreeFunc free_func, void *data);

#endif
