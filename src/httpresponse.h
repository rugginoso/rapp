/*
 * httpresponse.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "rapp/rapp_httpresponse.h"

struct Logger;
struct TcpConnection;

struct HTTPResponse* http_response_new(struct Logger *logger, const char *server_name);
void http_response_destroy(struct HTTPResponse *response);

void http_response_set_last(struct HTTPResponse *response, int last);
int http_response_is_last(struct HTTPResponse *response);

ssize_t http_response_read_data(struct HTTPResponse *response, void *data, size_t length);

#endif /* HTTTPRESPONSE_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

