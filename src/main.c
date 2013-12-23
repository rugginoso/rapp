/*
 * main.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "eloop.h"
#include "httprouter.h"
#include "httpserver.h"
#include "signalhandler.h"
#include "container.h"

static void
on_signal(struct SignalHandler *signal_handler,
          void                 *data)
{
  struct ELoop *eloop = NULL;

  assert(data != NULL);

  eloop = (struct ELoop *)data;

  event_loop_stop(eloop);
}

int
main(int argc, char *argv[])
{
  struct Logger *logger = NULL;
  struct ELoop *eloop = NULL;
  struct HTTPRouter *http_router = NULL;
  struct HTTPServer *http_server = NULL;
  struct SignalHandler *signal_handler = NULL;
  struct Container *container = NULL;
  enum RouteMatchMode match_mode = ROUTE_MATCH_FIRST;

  if (argc != 2) {
    logger_panic("usage: %s path_to_container");
    exit(255);
  }

  logger = logger_new_console(LOG_LAST, stderr);
  if (logger == NULL) {
    logger_panic("failed to initialize the logger!");
    exit(1);
  }

  container = container_new(logger, argv[1], 0, NULL); // FIXME
  if (!container) {
    exit(1);
  }

  logger_trace(logger, LOG_INFO, "rapp",
               "rapp %s (rev %s - tag: %s) starting... (PID=%d)",
               rapp_get_version(), rapp_get_version_sha1(),
               rapp_get_version_tag(), getpid());

  eloop = event_loop_new(logger);

  signal_handler = signal_handler_new(logger, eloop);
  signal_handler_add_signal_callback(signal_handler, SIGINT, on_signal, eloop);
  signal_handler_add_signal_callback(signal_handler, SIGTERM, on_signal, eloop);

  http_router = http_router_new(logger, match_mode);
  http_router_bind(http_router, "/", container);

  http_server = http_server_new(logger, eloop, http_router);

  http_server_start(http_server, "127.0.0.1", 8000);

  event_loop_run(eloop);

  http_server_destroy(http_server);
  http_router_destroy(http_router);
  signal_handler_destroy(signal_handler);
  event_loop_destroy(eloop);

  container_destroy(container);

  logger_trace(logger, LOG_INFO, "rapp",
               "%s", "rapp finished!");

  logger_destroy(logger);

  return 0;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

