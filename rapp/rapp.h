#ifndef RAPP_H
#define RAPP_H

#include "rapp_httprequest.h"
#include "rapp_httpresponse.h"

// Increment this on introdution of ABI incompatibilities.
#define ABI_VERSION 1

int rapp_get_abi_version(void);

// FIXME until we settle on the configuration handling
void *rapp_create(int ac, char **av, int *err);

int rapp_destroy(void *handle);

int rapp_serve(void *handle,
               struct HTTPRequest *http_request,
               struct HTTPResponseWriter *response_writer);

#endif /* RAPP_H */

