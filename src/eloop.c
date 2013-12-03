#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/epoll.h>

#include "eloop.h"

#define MAX_EVENTS 10


struct ELoop {
  int epollfd;
  struct Collector *collector;
  struct ELoopCallback *callbacks_list;
  int running;
};

struct ELoopCallback {
  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
  int fd;
  const void *data;

  struct ELoopCallback *next;
};


struct ELoop *
event_loop_new(void)
{
  struct ELoop *eloop = NULL;

  if ((eloop = calloc(sizeof(struct ELoop), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  if ((eloop->epollfd = epoll_create(1)) < 0) {
    perror("epoll_create");
    return NULL;
  }

  eloop->collector = collector_new();
  eloop->callbacks_list = NULL;

  return eloop;
}

void
event_loop_destroy(struct ELoop *eloop)
{
  assert(eloop != NULL);

  if (eloop->collector)
    collector_destroy(eloop->collector);

  close(eloop->epollfd);
  free(eloop);
}

int
event_loop_run(struct ELoop *eloop)
{
  int nfds = 0;
  int i = 0;
  struct ELoopCallback *eloop_callback = NULL;
  struct epoll_event events[MAX_EVENTS];

  eloop->running = 1;

  while(eloop->running && ((nfds = epoll_wait(eloop->epollfd, events, MAX_EVENTS, -1)) > -1)) {
    for (i = 0; i < nfds; i++) {
      eloop_callback = events[i].data.ptr;

      if (events[i].events & EPOLLIN && eloop_callback->callbacks[ELOOP_CALLBACK_READ]) {
        eloop_callback->callbacks[ELOOP_CALLBACK_READ](eloop_callback->fd, eloop_callback->data);
      }

      if (events[i].events & EPOLLOUT && eloop_callback->callbacks[ELOOP_CALLBACK_WRITE]) {
        eloop_callback->callbacks[ELOOP_CALLBACK_WRITE](eloop_callback->fd, eloop_callback->data);
      }

      if (events[i].events & EPOLLRDHUP && eloop_callback->callbacks[ELOOP_CALLBACK_CLOSE]) {
        eloop_callback->callbacks[ELOOP_CALLBACK_CLOSE](eloop_callback->fd, eloop_callback->data);
      }
    }
    collector_collect(eloop->collector);
  }

  return -1;
}

void
event_loop_stop(struct ELoop *eloop)
{
  eloop->running = 0;
}

int
event_loop_add_fd_watch(struct ELoop         *eloop,
                        int                   fd,
                        ELoopWatchFdCallback  callbacks[ELOOP_CALLBACK_MAX],
                        const void           *data)
{
  struct epoll_event ev = {0,};
  struct ELoopCallback *eloop_callback;
  int i = 0;

  assert(eloop != NULL);
  assert(fd > -1);
  assert(callbacks != NULL);

  for (; i < ELOOP_CALLBACK_MAX; i++) {
    if (callbacks[i]) {
      switch (i) {
      case ELOOP_CALLBACK_READ:
        ev.events |= EPOLLIN;
        break;
      case ELOOP_CALLBACK_WRITE:
        ev.events |= EPOLLOUT;
        break;
      case ELOOP_CALLBACK_CLOSE:
        ev.events |= EPOLLRDHUP;
        break;
      }
    }
  }

  /*
   * FIXME: verify it is the desired behaviour
   */
  ev.events |= EPOLLET;

  if ((eloop_callback = calloc(sizeof(struct ELoopCallback), 1)) == NULL) {
    perror("calloc");
    return -1;
  }

  memcpy(eloop_callback->callbacks, callbacks, ELOOP_CALLBACK_MAX * sizeof(ELoopWatchFdCallback));
  eloop_callback->fd = fd;
  eloop_callback->data = data;

  /*
   * Prepend this callback to the callback_list
   */
  eloop_callback->next = eloop->callbacks_list;
  eloop->callbacks_list = eloop_callback;

  ev.data.ptr = eloop_callback;

  return epoll_ctl(eloop->epollfd, EPOLL_CTL_ADD, fd, &ev);
}

int
event_loop_remove_fd_watch(struct ELoop *eloop,
                           int    fd)
{
  struct ELoopCallback *cb, *pcb = NULL;

  assert(eloop != NULL);
  assert(fd > -1);

  for (cb = eloop->callbacks_list; cb != NULL; pcb = cb, cb = cb->next) {
    if (cb->fd == fd) {
      if (pcb == NULL)
        eloop->callbacks_list = cb->next;
      else
        pcb->next = cb->next;
      free(cb);
      break;
    }
  }

  return epoll_ctl(eloop->epollfd, EPOLL_CTL_DEL, fd, NULL);
}

void
event_loop_schedule_free(struct ELoop     *eloop,
                         CollectorFreeFunc free_func,
                         void             *data)
{
  assert(eloop != NULL);
  assert(free_func != NULL);
  assert(data != NULL);

  collector_schedule_free(eloop->collector, free_func, data);
}

