/*
 * httprequest.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "rapp/rapp_httprequest.h"


typedef void (*HTTPRequestParseFinishCallback)(struct HTTPRequest *request, void *data);

struct Logger;

struct HTTPRequest *http_request_new(struct Logger *logger);
void http_request_destroy(struct HTTPRequest *request);

void http_request_set_parse_finish_callback(struct HTTPRequest *request, HTTPRequestParseFinishCallback callback, void *data);

int http_request_append_data(struct HTTPRequest *request, void *data, size_t length);

struct HTTPRequest *http_request_new_fake_url(struct Logger *logger, const char *url);

#endif /* HTTPREQUEST_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

