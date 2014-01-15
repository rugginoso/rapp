/*
 * rapp.h - is part of the public API of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_H
#define RAPP_H

#include "rapp_httprequest.h"
#include "rapp_httpresponse.h"
#include "rapp_logger.h"
#include "rapp_version.h"
#include "rapp_config.h"

struct RappContainer;

/* on the main thread */
struct RappContainer *rapp_create(void *cookie, struct RappConfig *config, int *err);
int rapp_destroy(struct RappContainer *handle);

/* on the container thread */
/* gets the up-to-date config. Intialize VM/TLS data here. */
int rapp_setup(struct RappContainer *handle, struct RappConfig *config);
int rapp_teardown(struct RappContainer *handle);

int rapp_serve(struct RappContainer *handle, struct HTTPRequest *request, struct HTTPResponse *response);

#endif /* RAPP_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

