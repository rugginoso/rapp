/*
 * httpconnection.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

struct Logger;
struct TcpConnection;
struct HTTPConnection;
struct HTTPRouter;


typedef void (*HTTPConnectionFinishCallback)(struct HTTPConnection *connection, void *data);

struct HTTPConnection *http_connection_new(struct Logger *logger, struct TcpConnection *tcp_connection, struct HTTPRouter *router);
void http_connection_destroy(struct HTTPConnection *http_connection);

void http_connection_set_finish_callback(struct HTTPConnection *connection, HTTPConnectionFinishCallback finish_callback, void *data);

#endif /* HTTPCONNECTION_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

