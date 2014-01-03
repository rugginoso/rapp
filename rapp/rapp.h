/*
 * rapp.h - is part of the public API of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
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

struct RappContainer *rapp_create(void *cookie, struct RappConfig *config, int *err);

int rapp_destroy(struct RappContainer *handle);
int rapp_init(struct RappContainer *handle, struct RappConfig *config);

int rapp_serve(struct RappContainer *handle,
               struct HTTPRequest *http_request,
               struct HTTPResponse *response);

#endif /* RAPP_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

