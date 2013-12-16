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
#include "config_private.h"

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

  config = config_new(logger);
  if (!config) {
    logger_destroy(logger);
    exit(1);
  }

  char *address;
  long port;
  assert(config_opt_add(config, "core", "address", PARAM_STRING, "Address to listen to") == 0);
  assert(config_opt_add(config, "core", "port", PARAM_INT, "Port") == 0);
  assert(config_opt_add(config, "core", "loglevel", PARAM_INT, "Verbosity level") == 0);
  assert(config_opt_add(config, "core", "test", PARAM_STRING, "Multi test") == 0);
  assert(config_opt_add(config, "other", "test", PARAM_STRING, "Test option in different section") == 0);

  assert(config_opt_set_range_int(config, "core", "port", 0, 65535) == 0);
  assert(config_opt_set_multivalued(config, "core", "test", 1) == 0);
  assert(config_opt_set_default_int(config, "core", "port", 8080) == 0);
  assert(config_opt_set_default_string(config, "core", "address", "127.0.0.1") == 0);
  assert(config_opt_set_default_int(config, "core", "loglevel", LOG_INFO) == 0);

  assert(config_generate_commandline(config) == 0);
  assert(config_parse(config, "example.yaml") == 0);

  config_get_string(config, "core", "address", &address);
  config_get_int(config, "core", "port", &port);
  logger_trace(logger, LOG_INFO, "rapp", "listening on %s", address);
  logger_trace(logger, LOG_INFO, "rapp", "listening on %d", port);

  container = container_new(logger, argv[1], 0, NULL); // FIXME
  if (!container) {
    config_destroy(config);
    logger_destroy(logger);
    free(address);
    exit(1);
  }

  logger_trace(logger, LOG_INFO, "rapp",
               "rapp %s (rev %s - tag: %s) starting... (PID=%li)",
               rapp_get_version(), rapp_get_version_sha1(),
               rapp_get_version_tag(), getpid());

  eloop = event_loop_new();

  signal_handler = signal_handler_new(eloop);
  signal_handler_add_signal_callback(signal_handler, SIGINT, on_signal, eloop);
  signal_handler_add_signal_callback(signal_handler, SIGTERM, on_signal, eloop);

  http_server = http_server_new(eloop);

  http_server_start(http_server, address, port);
  free(address);

  event_loop_run(eloop);

  http_server_destroy(http_server);
  signal_handler_destroy(signal_handler);
  event_loop_destroy(eloop);

  container_destroy(container);

  config_destroy(config);

  logger_trace(logger, LOG_INFO, "rapp",
               "%s", "rapp finished!");

  logger_destroy(logger);

  return 0;
}

