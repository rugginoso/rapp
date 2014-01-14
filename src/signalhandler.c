/*
 * signalhandler.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <bits/signum.h>
#include <sys/signalfd.h>

#include "eloop.h"
#include "logger.h"
#include "memory.h"
#include "signalhandler.h"


struct SignalHandler {
  int fd;
  sigset_t sigmask;

  struct ELoop *eloop;
  struct Logger *logger;

  SignalHandlerCallback callbacks[_NSIG];
  void *data[_NSIG];
};

static int
on_signal(int         fd,
          const void *data)
{
  SignalHandlerCallback callback = NULL;
  struct SignalHandler *signal_handler = NULL;
  struct signalfd_siginfo siginfo;

  assert(data != NULL);

  signal_handler = (struct SignalHandler *)data;

  if (read(fd, &siginfo, sizeof(struct signalfd_siginfo)) < 0) {
    LOGGER_PERROR(signal_handler->logger, "read");
    return -1;
  }

  callback = signal_handler->callbacks[siginfo.ssi_signo];
  if (callback != NULL) {
    void *data = signal_handler->data[siginfo.ssi_signo];
    callback(signal_handler, data);
  }
  return 0;
}

struct SignalHandler *
signal_handler_new(struct Logger *logger,
                   struct ELoop  *eloop)
{
  struct SignalHandler *signal_handler = NULL;

  if ((signal_handler = memory_create(sizeof(struct SignalHandler))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  sigemptyset(&(signal_handler->sigmask));

  if ((signal_handler->fd = signalfd(-1, &(signal_handler->sigmask), 0)) < 0) {
    LOGGER_PERROR(logger, "signalfd");
    memory_destroy(signal_handler);
    return NULL;
  }

  if (event_loop_add_fd_watch(eloop, signal_handler->fd, ELOOP_CALLBACK_READ, on_signal, signal_handler) < 0) {
    /* error already logged */
    close(signal_handler->fd);
    memory_destroy(signal_handler);
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

  memory_destroy(signal_handler);
}

static int
signal_handler_setup(struct SignalHandler *signal_handler,
                     unsigned              sig,
                     SignalHandlerCallback callback,
                     void                 *data)
{
  if (pthread_sigmask(SIG_SETMASK, &(signal_handler->sigmask), NULL) != 0) {
    LOGGER_PERROR(signal_handler->logger, "sigprocmask");
    return -1;
  }

  if (signalfd(signal_handler->fd, &(signal_handler->sigmask), 0) < 0) {
    LOGGER_PERROR(signal_handler->logger, "signalfd");
    return -1;
  }

  signal_handler->callbacks[sig] = callback;
  signal_handler->data[sig] = data;

  return 0;
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
  return signal_handler_setup(signal_handler, sig, callback, data);
}

int
signal_handler_remove_signal_callback(struct SignalHandler *signal_handler,
                                      unsigned              sig)
{
  assert(signal_handler != NULL);
  assert(sig < _NSIG);

  sigdelset(&(signal_handler->sigmask), sig);
  return signal_handler_setup(signal_handler, sig, NULL, NULL);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

