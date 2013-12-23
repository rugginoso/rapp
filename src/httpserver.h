/*
 * httpserver.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <inttypes.h>

struct Logger;
struct ELoop;
struct HTTPRouter;
struct HTTPServer;

struct HTTPServer *http_server_new(struct Logger *logger, struct ELoop *eloop, struct HTTPRouter *router);
void http_server_destroy(struct HTTPServer *http_server);

int http_server_start(struct HTTPServer *http_server, const char *host, uint16_t port);

#endif /* HTTPSERVER_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

