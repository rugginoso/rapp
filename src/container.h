#ifndef CONTAINER_H
#define CONTAINER_H

#include "logger.h"

#include "rapp/rapp.h"


typedef int (*RappServeCallback)(struct RappContainer *handle, struct HTTPRequest *http_request, struct HTTPResponseWriter *response_writer);

typedef int (*RappDestroyCallback)(struct RappContainer *handle);

struct Container;

struct Container *container_new(struct Logger *logger, const char *name, int ac, char **av);
void container_destroy(struct Container *container);

int container_serve(struct Container *container, struct HTTPRequest *http_request, struct HTTPResponseWriter *response_writer);

struct Container *container_new_null(struct Logger *logger, const char *tag);
struct Container *container_new_custom(struct Logger *logger, const char *tag, RappServeCallback serve, RappDestroyCallback destroy, void *user_data);

#endif /* CONTAINER_H */
