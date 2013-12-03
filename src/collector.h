#ifndef COLLECTOR_H
#define COLLECTOR_H

struct Collector;

typedef void (*CollectorFreeFunc)(void *data);

struct Collector *collector_new(void);
void collector_destroy(struct Collector *collector);

void collector_schedule_free(struct Collector *collector, CollectorFreeFunc, void *data);

void collector_collect(struct Collector *collector);

#endif /* COLLECTOR_H */

