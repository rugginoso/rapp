#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <bits/signum.h>
#include <sys/signalfd.h>

#include "eloop.h"
#include "signalhandler.h"


struct SignalHandler {
  int fd;
  sigset_t sigmask;

  struct ELoop *eloop;

  SignalHandlerCallback callbacks[_NSIG];
  void *data;
};

static int
on_signal(int         fd,
          const void *data)
{
  struct SignalHandler *signal_handler = NULL;
  struct signalfd_siginfo siginfo;
  ssize_t got = -1;

  assert(data != NULL);

  signal_handler = (struct SignalHandler *)data;

  if (read(fd, &siginfo, sizeof(struct signalfd_siginfo)) < 0) {
    perror("read");
    return -1;
  }

  if (signal_handler->callbacks[siginfo.ssi_signo] != NULL)
    signal_handler->callbacks[siginfo.ssi_signo](signal_handler, signal_handler->data);

  return 0;
}

struct SignalHandler *
signal_handler_new(struct ELoop *eloop)
{
  struct SignalHandler *signal_handler = NULL;
  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];

  if ((signal_handler = calloc(sizeof(struct SignalHandler), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  sigemptyset(&(signal_handler->sigmask));

  if ((signal_handler->fd = signalfd(-1, &(signal_handler->sigmask), 0)) < 0) {
    perror("signalfd");
    free(signal_handler);
    return NULL;
  }

  callbacks[ELOOP_CALLBACK_READ] = on_signal;
  callbacks[ELOOP_CALLBACK_WRITE] = NULL;
  callbacks[ELOOP_CALLBACK_CLOSE] = NULL;

  if (event_loop_add_fd_watch(eloop, signal_handler->fd, callbacks, signal_handler) < 0) {
    perror("epoll_ctl");
    close(signal_handler->fd);
    free(signal_handler);
    return NULL;
  }

  signal_handler->eloop = eloop;

  return signal_handler;
}

void
signal_handler_destroy(struct SignalHandler *signal_handler)
{
  assert(signal_handler != NULL);

  if (signal_handler->fd > 0) {
    event_loop_remove_fd_watch(signal_handler->eloop, signal_handler->fd);
    close(signal_handler->fd);
  }

  free(signal_handler);
}

int
signal_handler_add_signal_callback(struct SignalHandler *signal_handler,
                                   unsigned              sig,
                                   SignalHandlerCallback callback,
                                   void                 *data)
{
  assert(signal_handler != NULL);
  assert(sig < _NSIG);

  sigaddset(&(signal_handler->sigmask), sig);

  if (sigprocmask(SIG_SETMASK, &(signal_handler->sigmask), NULL) != 0) {
    perror("sigprocmask");
    return -1;
  }

  if (signalfd(signal_handler->fd, &(signal_handler->sigmask), 0) < 0) {
    perror("signalfd");
    return -1;
  }

  signal_handler->callbacks[sig] = callback;
  signal_handler->data = data;

  return 0;
}

int
signal_handler_remove_signal_callback(struct SignalHandler *signal_handler,
                                      unsigned              sig)
{
  assert(signal_handler != NULL);
  assert(sig < _NSIG);

  sigdelset(&(signal_handler->sigmask), sig);

  if (sigprocmask(SIG_SETMASK, &(signal_handler->sigmask), NULL) != 0) {
    perror("sigprocmask");
    return -1;
  }

  if (signalfd(signal_handler->fd, &(signal_handler->sigmask), 0) < 0) {
    perror("signalfd");
    return -1;
  }

  signal_handler->callbacks[sig] = NULL;

  return 0;
}
