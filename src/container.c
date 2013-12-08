#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <dlfcn.h>

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
is_ABI_compatible(int core_abi, int plugin_abi)
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


typedef void *(*PluginCreateFunc)(void * cookie, int ac, char **av, int *err);

static struct Container *
container_make(void          *plugin,
               struct Logger *logger,
               const char    *name,
               int            ac,
               char         **av)
{
  struct Container *container = NULL;
  PluginCreateFunc plugin_create = dlsym(plugin, "rapp_create");
  if (plugin_create) {
    int err = 0;
    container = calloc(1, sizeof(struct Container));
    if (container) {
      struct RappContainer *handle = plugin_create(container, ac, av, &err);
      if (handle) {
        container->name = strdup(name);
        container->logger = logger;
        container->plugin = plugin;
        container->handle = handle;
        container->serve = dlsym(plugin, "rapp_serve");
        container->destroy = dlsym(plugin, "rapp_destroy");

        logger_trace(logger, LOG_INFO, "loader", "loaded plugin[%s] id=%p (%p)", container->name, container, container->plugin);
      }
    } else {
      logger_trace(logger, LOG_ERROR, "loader", "plugin[%s] creation failed error=%i", name, err);
    }
  } else {
    logger_trace(logger, LOG_ERROR, "loader", "missing symbol in plugin[%s]: %s", name, dlerror());
  }
  return container;
}


typedef int (*PluginGetAbiVersionFunc)(void);

struct Container *
container_new(struct Logger *logger, const char *name, int ac, char **av)
{
  struct Container *container = NULL;
  void *plugin = NULL;
  
  logger_trace(logger, LOG_INFO, "loader",
               "loading plugin[%s]", name);

  plugin = dlopen(name, RTLD_NOW);
  if (plugin) {
    PluginGetAbiVersionFunc plugin_get_abi_version = dlsym(plugin, "rapp_get_abi_version");
    if (plugin_get_abi_version) {
      int plugin_abi = plugin_get_abi_version();
      if (is_ABI_compatible(ABI_VERSION, plugin_abi)) {
        container = container_make(plugin, logger, name, ac, av);
      } else {
        logger_trace(logger, LOG_ERROR, "loader", "ABI mismatch: core=%i plugin[%s]=%i", ABI_VERSION, name, plugin_abi);
        dlclose(plugin);
      }
    } else {
      logger_trace(logger, LOG_ERROR, "loader", "missing symbol in plugin[%s]: %s", name, dlerror());
      dlclose(plugin);
    }
  } else {
    logger_trace(logger, LOG_ERROR, "loader", "%s", dlerror());
  }
  return container;
}

void
container_destroy(struct Container *container)
{
  int err = 0;

  assert(container != NULL);

  logger_trace(container->logger, LOG_INFO, "loader", "unloading plugin[%s] id=%p (%p)", container->name, container, container->plugin);

  err = container->destroy(container->handle);
  if (!err) {
      dlclose(container->plugin);

      logger_trace(container->logger, LOG_INFO, "loader", "unloaded plugin[%s]", container->name);
      free(container->name);
      free(container);  // caveat emptor!
  }
}

void
container_serve(struct Container          *container,
                struct HTTPRequest        *http_request,
                struct HTTPResponseWriter *response_writer)
{
  assert(container != NULL);
  assert(http_request != NULL);
  assert(response_writer != NULL);

  container->serve(container->handle, http_request, response_writer);
}

