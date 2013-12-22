/*
 * httprouter.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "httprouter.h"
#include "container.h"

#define ROUTE_BASE_LEN 16
#define ROUTE_GROUP_LEN 30

struct RouteBinding {
  char name[ROUTE_BASE_LEN];
  char *route;
  struct Container *container;
};

/*
 * keep sizeof() <= 1024
 * when tune constants, remember to optimize
 * for a cacheline of 64 bytes
 */
struct RoutePack {
  struct RouteBinding bindings[ROUTE_GROUP_LEN];
  struct RoutePack *next;
  int32_t binding_num;
};

struct HTTPRouter {
  struct RoutePack pack;
  struct RoutePack *last;
  struct Logger *logger;
  struct Container *null;
  struct Container *starter; /* because `default` is a keyword. */
};

struct HTTPRouter *
http_router_new(struct Logger *logger)
{
  struct HTTPRouter *router = NULL;
  struct Container *null = NULL;

  assert(logger);

  if ((null = container_new_null(logger, "default")) == NULL) {
    /* TODO trace */
    return NULL;
  }

  if ((router = calloc(1, sizeof(struct HTTPRouter))) == NULL) {
    perror("calloc");
    container_destroy(null);
    return NULL;
  }

  router->logger = logger;
  router->null = null;
  router->last = &router->pack;

  return router;
}

static void
route_pack_clean(struct RoutePack *pack)
{
  int i = 0;

  assert(pack);

  for (i = 0; i < pack->binding_num; i++)
    if (pack->bindings[i].route != pack->bindings[i].name)
      free(pack->bindings[i].route);
}

/*
 * can be invoked against NULL packs,
 * so we need something lighter than an assert()
 */
static void
route_pack_destroy(struct RoutePack *pack)
{
  struct RoutePack *next = NULL;

  if (pack == NULL)
    return;

  next = pack->next;
  route_pack_clean(pack);
  free(pack);
  route_pack_destroy(next);
}

static struct RoutePack *
route_pack_bind(struct RoutePack *pack,
                const char       *route,
                struct Container *container)
{
  struct RoutePack *last = NULL;

  assert(pack);

  if (pack->binding_num == ROUTE_GROUP_LEN) {
    pack->next = calloc(1, sizeof(struct RoutePack));
    last = route_pack_bind(pack->next, route, container);
  } else {
    size_t len = strlen(route);
    struct RouteBinding *binding = &(pack->bindings[pack->binding_num]);
    if (len < ROUTE_BASE_LEN) {
      strcpy(binding->name, route);
      binding->route = binding->name;
    } else {
      binding->route = strdup(route);
    }
    binding->container = container;
    pack->binding_num++;
    last = pack;
  }
  return last;
}

void
http_router_destroy(struct HTTPRouter *router)
{
  assert(router);

  container_destroy(router->null);
  route_pack_clean(&router->pack);
  route_pack_destroy(router->pack.next);
  free(router);
}

int
http_router_set_default_container(struct HTTPRouter *router,
                                 struct Container   *container)
{
  assert(router);
  assert(container);

  router->starter = container;

  return 0;
}

int
http_router_bind(struct HTTPRouter *router,
                 const char        *route,
                 struct Container  *container)
{
  assert(router);
  assert(route);
  assert(container);

  router->last = route_pack_bind(router->last, route, container);

  return 0;
}

/*
 * 0 -> no match; > 0 = len of common string
 * checks at most 'n' chars.
 */
static int
strmatch(const char *s1,
         const char *s2,
         int         n)
{
  int match = 0;
  while (n-- && s1 && s2 && (*s1++ == *s2++)) {
    match++;
  }
  return match;
}

int
http_router_serve(struct HTTPRouter         *router,
                  struct HTTPRequest        *request,
                  struct HTTPResponseWriter *response_writer)
{
  const char *raw_req = NULL;
  struct Container *container = NULL;
  struct RoutePack *pack;
  struct MemoryRange uri_range;
  int found = 0;

  assert(router);
  assert(request);
  assert(response_writer);

  if (router->starter) {
    int ret = container_serve(router->starter, request, response_writer);
    if (ret == 0)
      return ret;
  }

  container = router->null;
  raw_req = http_request_get_buffer(request);
  http_request_get_url_range(request, &uri_range);
  for (pack = &router->pack; !found && pack != NULL; pack = pack->next) {
    int i = 0;
    for (i = 0; !found && i < pack->binding_num; i++) {
      int current = strmatch(pack->bindings[i].route, raw_req + uri_range.offset, uri_range.length);
      if (current > 0) {
        container = pack->bindings[i].container;
        found = 1;
      }
    }
  }

  return container_serve(container, request, response_writer);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

