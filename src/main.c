#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include "eloop.h"
#include "httpserver.h"
#include "signalhandler.h"

static void
on_signal(struct SignalHandler *signal_handler, void *data)
{
  struct ELoop *eloop = NULL;

  assert(data != NULL);

  eloop = (struct ELoop *)data;

  event_loop_stop(eloop);
}

int
main(int argc, char *argv[])
{
  struct ELoop *eloop = NULL;
  struct HTTPServer *http_server = NULL;
  struct SignalHandler *signal_handler = NULL;

  eloop = event_loop_new();

  signal_handler = signal_handler_new(eloop);
  signal_handler_add_signal_callback(signal_handler, SIGINT, on_signal, eloop);
  signal_handler_add_signal_callback(signal_handler, SIGTERM, on_signal, eloop);

  http_server = http_server_new(eloop);

  http_server_start(http_server, "127.0.0.1", 8000);

  event_loop_run(eloop);

  http_server_destroy(http_server);
  signal_handler_destroy(signal_handler);
  event_loop_destroy(eloop);

  return 0;
}
