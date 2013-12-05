#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "rapp/rapp_httprequest.h"


typedef void (*HTTPRequestParseFinishCallback)(struct HTTPRequest *request, void *data);


struct HTTPRequest *http_request_new(void);
void http_request_destroy(struct HTTPRequest *request);

void http_request_set_parse_finish_callback(struct HTTPRequest *request, HTTPRequestParseFinishCallback callback, void *data);

int http_request_append_data(struct HTTPRequest *request, void *data, size_t length);


#endif /* HTTPREQUEST_H */
