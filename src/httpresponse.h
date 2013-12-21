/*
 * httpresponse.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "rapp/rapp_httpresponse.h"

struct TcpConnection;

typedef void (*HTTPResponseNotifyCallback)(struct HTTPResponse *response, void *data);


struct HTTPResponse* http_response_new(struct TcpConnection *tcp_connection, HTTPResponseNotifyCallback headers_sent_callback, HTTPResponseNotifyCallback body_sent_callback, void *data);
void http_response_destroy(struct HTTPResponse *response);

#endif /* HTTTPRESPONSE_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

