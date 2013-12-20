/*
 * container.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef CONTAINER_H
#define CONTAINER_H

#include "logger.h"

#include "rapp/rapp.h"


struct Container;

struct Container *container_new(struct Logger *logger, const char *name, int ac, char **av);
void container_destroy(struct Container *container);

void container_serve(struct Container *container, struct HTTPRequest *http_request, struct HTTPResponse *response);

#endif /* CONTAINER_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

