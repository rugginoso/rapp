/*
 * httpresponsewriter.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPRESPONSEWRITER_H
#define HTTPRESPONSEWRITER_H

#include "rapp/rapp_httpresponse.h"

struct TcpConnection;

typedef void (*HTTPResponseWriterNotifyCallback)(struct HTTPResponseWriter *response_writer, void *data);


struct HTTPResponseWriter* http_response_writer_new(struct TcpConnection *tcp_connection, HTTPResponseWriterNotifyCallback headers_sent_callback, HTTPResponseWriterNotifyCallback body_sent_callback, void *data);
void http_response_writer_destroy(struct HTTPResponseWriter *response_writer);

#endif /* HTTTPRESPONSEWRITER_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

