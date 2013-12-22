/*
 * httprouter.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPROUTER_H
#define HTTPROUTER_H

struct HTTPRequest;
struct HTTPResponseWriter;
struct Container;
struct Logger;

struct HTTPRouter;

struct HTTPRouter *http_router_new(struct Logger *logger);
void http_router_destroy(struct HTTPRouter *router);

int http_router_set_default_container(struct HTTPRouter *router, struct Container *container);

int http_router_bind(struct HTTPRouter *router, const char *route, struct Container *container);

int http_router_serve(struct HTTPRouter *router, struct HTTPRequest *request, struct HTTPResponseWriter *response_writer);

#endif /* HTTPROUTER_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

