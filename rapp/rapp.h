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
               struct HTTPResponseWriter *response_writer);

#endif /* RAPP_H */

