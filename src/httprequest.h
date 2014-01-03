/*
 * httprequest.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "rapp/rapp_httprequest.h"

struct Logger;

struct HTTPRequest *http_request_new(struct Logger *logger);
void http_request_destroy(struct HTTPRequest *request);

void http_request_set_method(struct HTTPRequest *request, enum HTTPMethod method);

void http_request_set_url_range(struct HTTPRequest *request, size_t offset, size_t length);
void http_request_set_url_field_range(struct HTTPRequest *request, enum HTTPURLField field, size_t offset, size_t length);

int http_request_set_headers_buffer(struct HTTPRequest *request, const char *buffer, size_t length);
void http_request_set_header_key_range(struct HTTPRequest *request, unsigned header_index, size_t offset, size_t length);
void http_request_set_header_value_range(struct HTTPRequest *request, unsigned header_index, size_t offset, size_t length);

int http_request_set_body_length(struct HTTPRequest *request, size_t length);
void http_request_append_body_data(struct HTTPRequest *request, const char *data, size_t length);

void http_request_set_last(struct HTTPRequest *request, int last);
int http_request_is_last(struct HTTPRequest *request);

struct HTTPRequest *http_request_new_fake_url(struct Logger *logger, const char *url);

#endif /* HTTPREQUEST_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

