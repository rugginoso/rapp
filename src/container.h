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


typedef int (*RappServeCallback)(struct RappContainer *handle, struct HTTPRequest *http_request, struct HTTPResponse *response);
typedef int (*RappInitCallBack)(struct RappContainer *handle, struct RappConfig *config);
typedef int (*RappDestroyCallback)(struct RappContainer *handle);

struct Container;

struct Container *container_new(struct Logger *logger, const char *name, struct RappConfig *config);
void container_destroy(struct Container *container);
int container_init(struct Container *container, struct RappConfig *config);
int container_serve(struct Container *container, struct HTTPRequest *http_request, struct HTTPResponse *response);

struct Container *container_new_null(struct Logger *logger, const char *tag);
struct Container *container_new_custom(struct Logger *logger, const char *tag, RappInitCallBack init, RappServeCallback serve, RappDestroyCallback destroy, void *user_data);

#endif /* CONTAINER_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

