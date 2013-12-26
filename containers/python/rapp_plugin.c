/*
 * rapp_plugin.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <Python.h>

#include "rapp/rapp.h"

struct RappContainer {
};

int
rapp_get_abi_version()
{
  return ABI_VERSION;
}

int
rapp_serve(struct RappContainer *handle,
           struct HTTPRequest   *http_request,
           struct HTTPResponse  *http_response)
{
  return 0;
}

int
rapp_destroy(struct RappContainer *handle)
{
  return 0;
}

struct RappContainer *
rapp_create(void  *cookie,
            int    argc,
            char **argv,
            int   *err)
{
  return NULL;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

