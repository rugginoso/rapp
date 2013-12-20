/*
 * rapp.h - is part of RApp.
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

struct RappContainer;

// FIXME until we settle on the configuration handling
struct RappContainer *rapp_create(void *cookie, int ac, char **av, int *err);

int rapp_destroy(struct RappContainer *handle);

int rapp_serve(struct RappContainer *handle,
               struct HTTPRequest *http_request,
               struct HTTPResponse *response);

#endif /* RAPP_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

