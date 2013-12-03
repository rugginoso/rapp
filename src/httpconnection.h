#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

struct TcpConnection;
struct HTTPConnection;

typedef void (*HTTPConnectionFinishCallback)(struct HTTPConnection *connection, void *data);

struct HTTPConnection *http_connection_new(struct TcpConnection *tcp_connection);
void http_connection_destroy(struct HTTPConnection *http_connection);

void http_connection_set_finish_callback(struct HTTPConnection *connection, HTTPConnectionFinishCallback finish_callback, void *data);

#endif /* HTTPCONNECTION_H */

