#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "eloop.h"
#include "httpserver.h"
#include "signalhandler.h"
#include "container.h"
#include "config.h"

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
  struct Logger *logger = NULL;
  struct ELoop *eloop = NULL;
  struct HTTPServer *http_server = NULL;
  struct SignalHandler *signal_handler = NULL;
  struct Container *container = NULL;
  struct Config *config = NULL;

  logger = logger_open_console(LOG_LAST, stderr);
  if (logger == NULL) {
    logger_panic("failed to initialize the logger!");
    exit(1);
  }

  config = config_new();
  if (!config) {
      exit(1);
  }

  config_param_add(config, "core", "address", PARAM_STRING, "Address to listen to");
  config_param_add(config, "core", "port", PARAM_INT, "Port");
  config_param_add(config, "core", "loglevel", PARAM_STRING, "Verbosity level");

  config_param_set_range_int(config, "port", "core", 0, 65535);

  config_add_value(config, "core", "address", "127.0.0.1");
  config_add_value(config, "core", "port", "8080");
  config_add_value(config, "core", "loglevel", "DEBUG");
  const char *address;
  long port;
  config_get_string(config, "core", "address", &address);
  config_get_int(config, "core", "port", &port);
  logger_trace(logger, LOG_INFO, "rapp", "listening on %s", address);
  logger_trace(logger, LOG_INFO, "rapp", "listening on %d", port);

  container = container_new(logger, argv[1], 0, NULL); // FIXME
  if (!container) {
    exit(1);
  }

  logger_trace(logger, LOG_INFO, "rapp",
               "rapp starting... (PID=%li)",
               getpid());

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

  container_destroy(container);

  logger_trace(logger, LOG_INFO, "rapp",
               "%s", "rapp finished!");

  logger_destroy(logger);

  return 0;
}

