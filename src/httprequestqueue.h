/*
 * httprequestqueue.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef HTTPREQUESTQUEUE_H
#define HTTPREQUESTQUEUE_H

struct Logger;
struct HTTPRequest;
struct HTTPRequestQueue;

typedef void (*HTTPRequestQueueNewRequestCallback)(struct HTTPRequestQueue *queue, void *data);

struct HTTPRequestQueue *http_request_queue_new(struct Logger *logger);
void http_request_queue_destroy(struct HTTPRequestQueue *queue);

void http_request_queue_set_new_request_callback(struct HTTPRequestQueue *queue, HTTPRequestQueueNewRequestCallback callback, void *data);

int http_request_queue_append_data(struct HTTPRequestQueue *queue, void *data, size_t length);

struct HTTPRequest *http_request_queue_get_next_request(struct HTTPRequestQueue *queue);

#endif /* HTTPREQUESTQUEUE_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

