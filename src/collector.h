/*
 * collector.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef COLLECTOR_H
#define COLLECTOR_H

struct Logger;
struct Collector;

typedef void (*CollectorFreeFunc)(void *data);

struct Collector *collector_new(struct Logger *logger);
void collector_destroy(struct Collector *collector);

void collector_schedule_free(struct Collector *collector, CollectorFreeFunc, void *data);

void collector_collect(struct Collector *collector);

#endif /* COLLECTOR_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

