/*
 * main.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
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
#include "config/common.h"

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
  struct RappConfig *config = NULL;
  enum RouteMatchMode match_mode = ROUTE_MATCH_FIRST;
  char *address;
  long port;
  int num, i, res;
  char *confpath;
  struct RappArguments arguments;

  config_parse_early_commandline(&arguments, argc, argv);

  if (!arguments.logoutput) {
    exit(1);
  }
  if (arguments.logoutput_is_console == 1)
    logger = logger_new_console(arguments.loglevel, arguments.logoutput);
  else
    logger = logger_new_file(arguments.loglevel, arguments.logoutput);

  if (logger == NULL) {
    logger_panic("failed to initialize the logger!");
    exit(1);
  }

  config = config_new(logger);
  if (!config) {
    logger_destroy(logger);
    exit(1);
  }

  if (arguments.container) {
    container = container_new(logger, arguments.container, config);
    if (!container) {
      config_destroy(config);
      logger_destroy(logger);
      exit(1);
    }
  }

  rapp_config_opt_add(config, "core", "address", PARAM_STRING, "Address to listen to", NULL);
  rapp_config_opt_add(config, "core", "port", PARAM_INT, "Port", NULL);
  rapp_config_opt_add(config, "core", "config", PARAM_STRING, "Path to yaml config", "FILE");
  rapp_config_opt_add(config, "core", "confd", PARAM_STRING, "Path to directory to scan for config", "DIR");

  rapp_config_opt_set_range_int(config, "core", "port", 0, 65535);
  rapp_config_opt_set_default_string(config, "core", "address", "127.0.0.1");
  rapp_config_opt_set_default_int(config, "core", "port", 8080);
  rapp_config_opt_set_multivalued(config, "core", "config", 1);
  rapp_config_opt_set_multivalued(config, "core", "confd", 1);

  res = config_parse_commandline(config, argc, argv);
  if (res != 0) {
      exit(1);
  }
  res = config_read_env(config);
  if (res != 0) {
    exit(1);
  }

  // Scan configuration directories
  rapp_config_get_num_values(config, "core", "confd", &num);
  for (i = 0; i < num; i++) {
    if (rapp_config_get_nth_string(config, "core", "confd", i, &confpath) != 0)
        exit(1);
    res = config_scan_directory(config, confpath, ".yaml");
    free(confpath);
    if (res != 0)
        exit(1);
  }

  // Parsing individual configs
  rapp_config_get_num_values(config, "core", "config", &num);
  for (i = 0; i < num; i++) {
    if (rapp_config_get_nth_string(config, "core", "config", i, &confpath) != 0)
        exit(1);
    res = config_parse(config, confpath);
    free(confpath);
    if (res != 0)
        exit(1);
  }

  if (!arguments.container) {
    logger_trace(logger, LOG_CRITICAL, "rapp", "No container provided.");
    exit(1);
  }

  container_init(container, config);
  rapp_config_get_string(config, "core", "address", &address);
  rapp_config_get_int(config, "core", "port", &port);

  logger_trace(logger, LOG_INFO, "rapp", "listening on %s", address);
  logger_trace(logger, LOG_INFO, "rapp", "listening on %d", port);

  logger_trace(logger, LOG_INFO, "rapp",
               "rapp %s (rev %s) starting... (PID=%d)",
               rapp_get_version(), rapp_get_version_sha1(), getpid());

  eloop = event_loop_new(logger);

  signal_handler = signal_handler_new(logger, eloop);
  signal_handler_add_signal_callback(signal_handler, SIGINT, on_signal, eloop);
  signal_handler_add_signal_callback(signal_handler, SIGTERM, on_signal, eloop);

  http_router = http_router_new(logger, match_mode);
  http_router_bind(http_router, "/", container);

  http_server = http_server_new(logger, eloop, http_router);

  http_server_start(http_server, address, port);
  free(address);
  free(arguments.container);

  event_loop_run(eloop);

  http_server_destroy(http_server);
  http_router_destroy(http_router);
  signal_handler_destroy(signal_handler);
  event_loop_destroy(eloop);

  container_destroy(container);

  config_destroy(config);

  logger_trace(logger, LOG_INFO, "rapp",
               "%s", "rapp finished!");

  logger_destroy(logger);

  return 0;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

