#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <inttypes.h>

struct ELoop;
struct HTTPServer;

struct HTTPServer *http_server_new(struct ELoop *eloop);
void http_server_destroy(struct HTTPServer *http_server);

int http_server_start(struct HTTPServer *http_server, const char *host, uint16_t port);

#endif /* HTTPSERVER_H */

