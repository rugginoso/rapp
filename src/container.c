/*
 * container.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <errno.h>

#include "container.h"


struct Container {
  void *plugin;
  struct RappContainer *handle;

  struct Logger *logger;
  char *name;

  RappServeCallback serve;
  RappDestroyCallback destroy;
};

static int
is_ABI_compatible(int core_abi,
                  int plugin_abi)
{
  return core_abi == plugin_abi;
}

struct Logger *
logger_get(void *cookie)
{
  assert(cookie);

  return ((struct Container *)cookie)->logger;
}


static int
get_symbol(struct Logger *logger,
           void          *handle,
           const char    *name,
           void         **symbol)
{
  char *error = NULL;

  assert(handle != NULL);
  assert(symbol != NULL);
  assert(logger != NULL);

  /* Clean errors */
  dlerror();

  *symbol = dlsym(handle, name);

  /* Check dlerror(), because NULL is a valid return value for dlsym() */
  if ((error = dlerror()) != NULL) {
    logger_trace(logger, LOG_ERROR, "loader", "missing symbol in plugin[%s]: %s", name, error);
    return -1;
  }

  return 0;
}


typedef void *(*PluginCreateFunc)(void *cookie, int ac, char **av, int *err);

static struct Container *
container_make(void          *plugin,
               struct Logger *logger,
               const char    *name,
               int            ac,
               char         **av)
{
  PluginCreateFunc plugin_create = NULL;
  struct Container *container = NULL;
  struct RappContainer *handle = NULL;
  int error = 0;

  assert(plugin != NULL);
  /* the caller must ensure the other parameters are valid */

  if (get_symbol(logger, plugin, "rapp_create", (void *)&plugin_create) != 0)
    return NULL;

  if ((container = calloc(1, sizeof(struct Container))) == NULL) {
    logger_trace(logger, LOG_ERROR, "loader", "plugin[%s] alloc failed error=%s", name, strerror(errno));
    return NULL;
  }

  container->logger = logger;
  container->plugin = plugin;

  if ((container->name = strdup(name)) == NULL) {
    free(container);
    logger_trace(logger, LOG_ERROR, "loader", "plugin[%s] name alloc failed error=%s", name, strerror(errno));
    return NULL;
  }

  if ((handle = plugin_create(container, ac, av, &error)) == NULL) {
    free(container->name);
    free(container);
    logger_trace(logger, LOG_ERROR, "loader", "plugin[%s] creation failed error=%i", name, error);
    return NULL;
  }

  container->handle = handle;
  container->serve = dlsym(plugin, "rapp_serve");
  container->destroy = dlsym(plugin, "rapp_destroy");

  logger_trace(logger, LOG_INFO, "loader", "loaded plugin[%s] id=%p (%p)", container->name, container, container->plugin);

  return container;
}


typedef int (*PluginGetAbiVersionFunc)(void);

struct Container *
container_new(struct Logger *logger,
              const char    *name,
              int            ac,
              char         **av)
{
  void *plugin = NULL;
  PluginGetAbiVersionFunc plugin_get_abi_version = NULL;
  int plugin_abi = 0;

  assert(logger != NULL);
  assert(name != NULL);

  logger_trace(logger, LOG_INFO, "loader",
               "loading plugin[%s]", name);

  if ((plugin = dlopen(name, RTLD_NOW)) == NULL) {
    logger_trace(logger, LOG_ERROR, "loader", "%s", dlerror());
    return NULL;
  }

  if (get_symbol(logger, plugin, "rapp_get_abi_version", (void *)&plugin_get_abi_version) != 0) {
    dlclose(plugin);
    return NULL;
  }

  plugin_abi = plugin_get_abi_version();

  if (!is_ABI_compatible(ABI_VERSION, plugin_abi)) {
    logger_trace(logger, LOG_ERROR, "loader", "ABI mismatch: core=%i plugin[%s]=%i", ABI_VERSION, name, plugin_abi);
    dlclose(plugin);
    return NULL;
  }

  return container_make(plugin, logger, name, ac, av);
}

void
container_destroy(struct Container *container)
{
  assert(container != NULL);

  logger_trace(container->logger, LOG_INFO, "loader", "unloading plugin[%s] id=%p (%p)", container->name, container, container->plugin);

  if (container->destroy(container->handle) == 0) {
    /* the `null` container doesn't have a handle */
    if (container->plugin)
      dlclose(container->plugin);

    logger_trace(container->logger, LOG_INFO, "loader", "unloaded plugin[%s]", container->name);
    free(container->name);
    free(container);  /* caveat emptor! */
  }
}

int
container_serve(struct Container          *container,
                struct HTTPRequest        *http_request,
                struct HTTPResponse *response)
{
  assert(container != NULL);
  assert(http_request != NULL);
  assert(response != NULL);

  return container->serve(container->handle, http_request, response);
}

static int
null_serve(struct RappContainer      *handle,
           struct HTTPRequest        *request,
           struct HTTPResponse       *response)
{
  struct Logger *logger = NULL;

  assert(handle != NULL);
  assert(request != NULL);
  assert(response != NULL);

  logger = (struct Logger *)handle;
  logger_trace(logger, LOG_DEBUG, "null", "received request on unhandled route");
  return -1;
}

static int
null_destroy(struct RappContainer *handle)
{
  assert(handle);
  return 0;
}

struct Container *
container_new_custom(struct Logger      *logger,
                     const char         *tag,
                     RappServeCallback   serve,
                     RappDestroyCallback destroy,
                     void                *user_data)
{
  struct Container *container = NULL;
  if ((container = calloc(1, sizeof(struct Container))) == NULL) {
    LOGGER_PERROR(logger, "calloc");
    return NULL;
  }

  if ((container->name = strdup(tag)) == NULL) {
    LOGGER_PERROR(logger, "strdup");
    free(container);
    return NULL;
  }

  container->plugin = NULL;
  container->handle = user_data;
  container->logger = logger;
  container->serve = serve;
  container->destroy = destroy;
  return container;
}

struct Container *
container_new_null(struct Logger *logger,
                   const char    *tag)
{
  return container_new_custom(logger, tag, null_serve, null_destroy, logger);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

