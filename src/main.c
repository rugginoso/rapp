#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include "eloop.h"
#include "httpserver.h"

static int
on_signal(int fd, const void *data)
{
  struct ELoop *eloop = NULL;

  assert(data != NULL);

  eloop = (struct ELoop *)data;

  event_loop_stop(eloop);

  return 0;
}

int
main(int argc, char *argv[])
{
  struct ELoop *eloop = NULL;
  struct HTTPServer *http_server = NULL;
  int signal_fd = -1;
  sigset_t sig_mask;
  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];

  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGINT);
  sigaddset(&sig_mask, SIGTERM);

  if (sigprocmask(SIG_SETMASK, &sig_mask, NULL) != 0) {
    perror("sigprocmask");
    return -1;
  }

  if ((signal_fd = signalfd(-1, &sig_mask, 0)) < 0) {
    perror("signalfd");
    return -1;
  }

  eloop = event_loop_new();

  callbacks[ELOOP_CALLBACK_READ] = on_signal;
  callbacks[ELOOP_CALLBACK_WRITE] = NULL;
  event_loop_add_fd_watch(eloop, signal_fd, callbacks, eloop);

  http_server = http_server_new(eloop);

  http_server_start(http_server, "127.0.0.1", 8000);

  event_loop_run(eloop);

  event_loop_remove_fd_watch(eloop, signal_fd);
  close(signal_fd);

  http_server_destroy(http_server);
  event_loop_destroy(eloop);

  return 0;
}
