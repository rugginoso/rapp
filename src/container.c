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

  int (*serve)(void *, struct HTTPRequest *, struct HTTPResponseWriter *);

  int (*destroy)(void *);
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
  struct Logger *logger = NULL;

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
    logger_trace(logger, LOG_ERROR, "loader", "plugin[%s] creation failed error=%s", name, strerror(errno));
    return NULL;
  }

  if ((handle = plugin_create(container, ac, av, &error)) == NULL) {
    logger_trace(logger, LOG_ERROR, "loader", "plugin[%s] creation failed error=%i", name, error);
    free(container);
    return NULL;
  }

  container->name = strdup(name);
  container->logger = logger;
  container->plugin = plugin;
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
                struct HTTPResponseWriter *response_writer)
{
  assert(container != NULL);
  assert(http_request != NULL);
  assert(response_writer != NULL);

  return container->serve(container->handle, http_request, response_writer);
}


struct RappContainer;

static int
null_serve(void                      *handle,
           struct HTTPRequest        *request,
           struct HTTPResponseWriter *response_writer)
{
  struct Container *container = handle;
  logger_trace(container->logger, LOG_DEBUG, "null", "received request on unhandled route");
  return 0;
}

static int
null_destroy(void *handle)
{
  return 0;
}

struct Container *
container_null(struct Logger *logger)
{
  struct Container *container = NULL;
  if ((container = calloc(1, sizeof(struct Container))) == NULL)
    return NULL;

  container->plugin = NULL;
  container->handle = (struct RappContainer *)container; /* FIXME */
  container->logger = logger;
  container->name = strdup("null container");
  container->serve = null_serve;
  container->destroy = null_destroy;
  return container;
}

