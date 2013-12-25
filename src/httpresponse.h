/*
 * httpresponse.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "rapp/rapp_httpresponse.h"

struct Logger;
struct TcpConnection;

struct HTTPResponse* http_response_new(struct Logger *logger, const char *server_name);
void http_response_destroy(struct HTTPResponse *response);

ssize_t http_response_read_data(struct HTTPResponse *response, void *data, size_t length);

ssize_t http_response_sendfile(struct HTTPResponse *response, struct TcpConnection *tcp_connection);

#endif /* HTTTPRESPONSE_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

