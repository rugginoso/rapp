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
  char *address;
  long port;
  int num, i, res;
  char *confpath;

  logger = logger_new_console(LOG_LAST, stderr);
  if (logger == NULL) {
    logger_panic("failed to initialize the logger!");
    exit(1);
  }

  config = config_new(logger);
  if (!config) {
    logger_destroy(logger);
    exit(1);
  }

  assert(config_opt_add(config, "core", "address", PARAM_STRING, "Address to listen to", NULL) == 0);
  assert(config_opt_add(config, "core", "port", PARAM_INT, "Port", NULL) == 0);
  assert(config_opt_add(config, "core", "config", PARAM_STRING, "Path to yaml config", "FILE") == 0);
  assert(config_opt_add(config, "core", "confd", PARAM_STRING, "Path to directory to scan for config", "DIR") == 0);
  assert(config_opt_add(config, "core", "loglevel", PARAM_INT, "Verbosity level", NULL) == 0);

  assert(config_opt_set_range_int(config, "core", "port", 0, 65535) == 0);
  assert(config_opt_set_default_string(config, "core", "address", "127.0.0.1") == 0);
  assert(config_opt_set_default_int(config, "core", "port", 8080) == 0);
  assert(config_opt_set_default_int(config, "core", "loglevel", LOG_INFO) == 0);
  assert(config_opt_set_multivalued(config, "core", "config", 1) == 0);
  assert(config_opt_set_multivalued(config, "core", "confd", 1) == 0);

  assert(config_parse_commandline(config, argc, argv) == 0);

  // Scan configuration directories
  config_get_num_values(config, "core", "confd", &num);
  for (i = 0; i < num; i++) {
    if (config_get_nth_string(config, "core", "confd", i, &confpath) != 0)
        exit(1);
    res = config_scan_directory(config, confpath, ".yaml");
    free(confpath);
    if (res != 0)
        exit(1);
  }

  // Parsing individual configs
  config_get_num_values(config, "core", "config", &num);
  for (i = 0; i < num; i++) {
    if (config_get_nth_string(config, "core", "config", i, &confpath) != 0)
        exit(1);
    res = config_parse(config, confpath);
    free(confpath);
    if (res != 0)
        exit(1);
  }

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

