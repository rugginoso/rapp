/*
 * signalhandler.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

#include <bits/signum.h>
#include <sys/signalfd.h>

#include "logger.h"
#include "eloop.h"
#include "signalhandler.h"


struct SignalHandler {
  int fd;
  sigset_t sigmask;

  struct ELoop *eloop;
  struct Logger *logger;

  SignalHandlerCallback callbacks[_NSIG];
  void *data;
};

static int
on_signal(int         fd,
          const void *data)
{
  struct SignalHandler *signal_handler = NULL;
  struct signalfd_siginfo siginfo;

  assert(data != NULL);

  signal_handler = (struct SignalHandler *)data;

  if (read(fd, &siginfo, sizeof(struct signalfd_siginfo)) < 0) {
    logger_trace(signal_handler->logger, LOG_ERROR, "signalhandler", "read: %s", strerror(errno));
    return -1;
  }

  if (signal_handler->callbacks[siginfo.ssi_signo] != NULL)
    signal_handler->callbacks[siginfo.ssi_signo](signal_handler, signal_handler->data);

  return 0;
}

struct SignalHandler *
signal_handler_new(struct Logger *logger,
                   struct ELoop  *eloop)
{
  struct SignalHandler *signal_handler = NULL;

  if ((signal_handler = calloc(1, sizeof(struct SignalHandler))) == NULL) {
    logger_trace(logger, LOG_ERROR, "signalhandler", "calloc: %s", strerror(errno));
    return NULL;
  }

  sigemptyset(&(signal_handler->sigmask));

  if ((signal_handler->fd = signalfd(-1, &(signal_handler->sigmask), 0)) < 0) {
    logger_trace(logger, LOG_ERROR, "signalhandler", "signalfd: %s", strerror(errno));
    free(signal_handler);
    return NULL;
  }

  if (event_loop_add_fd_watch(eloop, signal_handler->fd, ELOOP_CALLBACK_READ, on_signal, signal_handler) < 0) {
    logger_trace(logger, LOG_ERROR, "signalhandler", "epoll_ctl: %s", strerror(errno));
    close(signal_handler->fd);
    free(signal_handler);
    return NULL;
  }

  signal_handler->logger = logger;
  signal_handler->eloop = eloop;

  return signal_handler;
}

void
signal_handler_destroy(struct SignalHandler *signal_handler)
{
  assert(signal_handler != NULL);

  if (signal_handler->fd > 0) {
    event_loop_remove_fd_watch(signal_handler->eloop, signal_handler->fd, ELOOP_CALLBACK_READ);
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
    logger_trace(signal_handler->logger, LOG_ERROR, "signalhandler", "sigprocmask: %s", strerror(errno));
    return -1;
  }

  if (signalfd(signal_handler->fd, &(signal_handler->sigmask), 0) < 0) {
    logger_trace(signal_handler->logger, LOG_ERROR, "signalhandler", "signalfd: %s", strerror(errno));
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
    logger_trace(signal_handler->logger, LOG_ERROR, "signalhandler", "sigprocmask: %s", strerror(errno));
    return -1;
  }

  if (signalfd(signal_handler->fd, &(signal_handler->sigmask), 0) < 0) {
    logger_trace(signal_handler->logger, LOG_ERROR, "signalhandler", "signalfd: %s", strerror(errno));
    return -1;
  }

  signal_handler->callbacks[sig] = NULL;

  return 0;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

