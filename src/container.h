/*
 * container.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef CONTAINER_H
#define CONTAINER_H

#include "logger.h"

#include "rapp/rapp.h"


typedef int (*RappServeCallback)(struct RappContainer *handle, struct HTTPRequest *request, struct HTTPResponse *response);
typedef int (*RappSetupCallBack)(struct RappContainer *handle, const struct RappConfig *config);
typedef int (*RappTeardownCallBack)(struct RappContainer *handle);
typedef int (*RappDestroyCallback)(struct RappContainer *handle);

struct Container;

struct Container *container_new(struct Logger *logger, const char *name, struct RappConfig *config);
void container_destroy(struct Container *container);
int container_run(struct Container *container, const struct RappConfig *config);
int container_serve(struct Container *container, struct HTTPRequest *request, struct HTTPResponse *response);

struct Container *container_new_null(struct Logger *logger, const char *tag);
struct Container *container_new_custom(struct Logger *logger, const char *tag, RappSetupCallBack setup, RappTeardownCallBack teardown, RappServeCallback serve, RappDestroyCallback destroy, void *user_data);

#endif /* CONTAINER_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

