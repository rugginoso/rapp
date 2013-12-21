/*
 * rapp_httpresponse.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_HTTPRESPONSE_H
#define RAPP_HTTPRESPONSE_H

#define HTTP_EOL "\r\n"

struct HTTPResponseWriter;

void http_response_writer_notify_headers_sent(struct HTTPResponseWriter *response_writer);
void http_response_writer_notify_body_sent(struct HTTPResponseWriter *response_writer);

ssize_t http_response_writer_printf(struct HTTPResponseWriter *response_writer, const char *fmt, ...);
ssize_t http_response_writer_write_data(struct HTTPResponseWriter *response_writer, const void *data, size_t length);
ssize_t http_response_writer_sendfile(struct HTTPResponseWriter *response_writer, const char *path);

#endif /* RAPP_HTTTPRESPONSE_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

