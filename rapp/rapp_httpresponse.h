/*
 * rapp_httpresponse.h - is part of the public API of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_HTTPRESPONSE_H
#define RAPP_HTTPRESPONSE_H

#define HTTP_EOL "\r\n"

struct HTTPResponse;

ssize_t http_response_write_status_line_by_code(struct HTTPResponse *response, unsigned code);
ssize_t http_response_write_status_line(struct HTTPResponse *response, const char *status_line);

ssize_t http_response_write_header(struct HTTPResponse *response, const char *key, const char *value);
ssize_t http_response_end_headers(struct HTTPResponse *response);

ssize_t http_response_append_data(struct HTTPResponse *response, const void *data, size_t length);

ssize_t http_response_write_error_by_code(struct HTTPResponse *response, unsigned code);

int http_response_write_file(struct HTTPResponse *response, const char *path, size_t length);

#endif /* RAPP_HTTTPRESPONSE_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

