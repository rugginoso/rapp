/*
 * collector.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "logger.h"
#include "collector.h"

struct CollectEntry {
  CollectorFreeFunc free_func;
  void *data;

  struct CollectEntry *next;
};

struct Collector {
  struct CollectEntry *collect_entries_list;
  struct Logger *logger;
};


struct Collector *
collector_new(struct Logger *logger)
{
  struct Collector *collector = NULL;

  if ((collector = calloc(1, sizeof(struct Collector))) == NULL) {
    logger_trace(logger, LOG_ERROR, "collector", "calloc: %s", strerror(errno));
    return NULL;
  }

  collector->logger = logger;

  return collector;
}

void
collector_destroy(struct Collector *collector)
{
  assert(collector != NULL);

  collector_collect(collector);

  free(collector);
}

void collector_schedule_free(struct Collector  *collector,
                             CollectorFreeFunc  free_func,
                             void              *data)
{
  struct CollectEntry *entry = NULL;

  assert(collector != NULL);
  assert(free_func != NULL);
  assert(data != NULL);

  // Look for already added object
  for (entry = collector->collect_entries_list; entry != NULL; entry = entry->next) {
    if (entry->data == data)
      return;
  }

  if ((entry = calloc(1, sizeof(struct CollectEntry))) == NULL) {
    logger_trace(collector->logger, LOG_ERROR, "collector", "calloc: %s", strerror(errno));
    return;
  }

  entry->free_func = free_func;
  entry->data = data;
  entry->next = collector->collect_entries_list;
  collector->collect_entries_list = entry;
}

void
collector_collect(struct Collector *collector)
{
  struct CollectEntry *next = NULL;

  assert(collector != NULL);

  while(collector->collect_entries_list != NULL) {
    next = collector->collect_entries_list->next;

    collector->collect_entries_list->free_func(collector->collect_entries_list->data);

    free(collector->collect_entries_list);

    collector->collect_entries_list = next;
  }
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

