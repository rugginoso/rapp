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
  int fd;
  struct epoll_event ev;
  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
  const void *datas[ELOOP_CALLBACK_MAX];

  struct ELoopCallback *next;
};


struct ELoop *
event_loop_new(void)
{
  struct ELoop *eloop = NULL;

  if ((eloop = calloc(1, sizeof(struct ELoop))) == NULL) {
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

  while(eloop->running && ((nfds = epoll_wait(eloop->epollfd, events, MAX_EVENTS, 100)) > -1)) {
    for (i = 0; i < nfds; i++) {
      eloop_callback = events[i].data.ptr;

      if (events[i].events & EPOLLIN && eloop_callback->callbacks[ELOOP_CALLBACK_READ]) {
        eloop_callback->callbacks[ELOOP_CALLBACK_READ](eloop_callback->fd, eloop_callback->datas[ELOOP_CALLBACK_READ]);
      }

      if (events[i].events & EPOLLOUT && eloop_callback->callbacks[ELOOP_CALLBACK_WRITE]) {
        eloop_callback->callbacks[ELOOP_CALLBACK_WRITE](eloop_callback->fd, eloop_callback->datas[ELOOP_CALLBACK_WRITE]);
      }

      if (events[i].events & EPOLLRDHUP && eloop_callback->callbacks[ELOOP_CALLBACK_CLOSE]) {
        eloop_callback->callbacks[ELOOP_CALLBACK_CLOSE](eloop_callback->fd, eloop_callback->datas[ELOOP_CALLBACK_CLOSE]);
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

static int
find_callback_by_fd(struct ELoop          *eloop,
                    int                    fd,
                    struct ELoopCallback **ec,
                    struct ELoopCallback **pec)
{
  assert(ec != NULL);

  for (*ec = eloop->callbacks_list; *ec != NULL; *ec = (*ec)->next) {
    if ((*ec)->fd == fd) {
      return 0;
    }

    if (pec != NULL)
      *pec = *ec;
  }

  return -1;
}

static uint32_t
eloop_callback_type_to_epoll_event(enum ELoopWatchFdCallbackType callback_type)
{
  uint32_t event = 0;

  switch (callback_type) {
  case ELOOP_CALLBACK_READ:
    event = EPOLLIN;
    break;
  case ELOOP_CALLBACK_WRITE:
    event = EPOLLOUT;
    break;
  case ELOOP_CALLBACK_CLOSE:
    event = EPOLLRDHUP;
    break;
  }

  return event;
}

int
event_loop_add_fd_watch(struct ELoop                 *eloop,
                        int                           fd,
                        enum ELoopWatchFdCallbackType callback_type,
                        ELoopWatchFdCallback          callback,
                        const void                   *data)
{
  struct ELoopCallback *ec = NULL;
  int epoll_operation = EPOLL_CTL_MOD;

  assert(eloop != NULL);
  assert(fd > -1);
  assert(callback != NULL);

  if (find_callback_by_fd(eloop, fd, &ec, NULL) < 0) {
    if ((ec = calloc(1, sizeof(struct ELoopCallback))) == NULL) {
      perror("calloc");
      return -1;
    }

    ec->next = eloop->callbacks_list;
    eloop->callbacks_list = ec;

    epoll_operation = EPOLL_CTL_ADD;
  }

  ec->ev.events |= eloop_callback_type_to_epoll_event(callback_type);
  ec->ev.data.ptr = ec;

  ec->fd = fd;
  ec->callbacks[callback_type] = callback;
  ec->datas[callback_type] = data;

  return epoll_ctl(eloop->epollfd, epoll_operation, fd, &(ec->ev));
}

int
event_loop_remove_fd_watch(struct ELoop                 *eloop,
                           int                           fd,
                           enum ELoopWatchFdCallbackType callback_type)
{
  struct ELoopCallback *ec = NULL;
  struct ELoopCallback *pec = NULL;
  int epoll_operation = EPOLL_CTL_MOD;
  int i = 0;

  assert(eloop != NULL);
  assert(fd > -1);

  if (find_callback_by_fd(eloop, fd, &ec, &pec) < 0)
    return -1;

  ec->callbacks[callback_type] = NULL;
  ec->datas[callback_type] = NULL;

  ec->ev.events &= ~(eloop_callback_type_to_epoll_event(callback_type));

  if (ec->ev.events == 0) {
    epoll_operation = EPOLL_CTL_DEL;

    if (pec == NULL)
      eloop->callbacks_list = ec->next;
    else
      pec->next = ec->next;
    free(ec);
  }

  return epoll_ctl(eloop->epollfd, epoll_operation, fd, &(ec->ev));
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

