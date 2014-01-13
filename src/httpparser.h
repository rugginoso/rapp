/*
 * httpparser.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

struct Logger;
struct HTTPRequest;
struct HTTPParser;

typedef void (*HTTPParserNewRequestCallback)(struct HTTPParser *parser, void *data);

struct HTTPParser *http_parser_new(struct Logger *logger);
void http_parser_destroy(struct HTTPParser *parser);

void http_parser_set_new_request_callback(struct HTTPParser *parser, HTTPParserNewRequestCallback callback, void *data);

int http_parser_append_data(struct HTTPParser *parser, void *data, size_t length);

struct HTTPRequest *http_parser_get_next_request(struct HTTPParser *parser);

#endif /* HTTPPARSER_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

