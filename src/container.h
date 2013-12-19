#ifndef CONTAINER_H
#define CONTAINER_H

#include "logger.h"

#include "rapp/rapp.h"


struct Container;

struct Container *container_new(struct Logger *logger, const char *name, int ac, char **av);
void container_destroy(struct Container *container);

int container_serve(struct Container *container, struct HTTPRequest *http_request, struct HTTPResponseWriter *response_writer);

struct Container *container_null(struct Logger *logger);

#endif /* CONTAINER_H */
