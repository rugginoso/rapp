#ifndef SYNCQUEUE_H
#define SYNCQUEUE_H

#include <sys/queue.h>

struct HTTPRequest;
struct HTTPResponse;
struct SyncQueue;

struct SyncQueueEntry {
  struct HTTPRequest *request;
  struct HTTPResponse *response;

  TAILQ_ENTRY(SyncQueueEntry) entries;
};

struct SyncQueue *sync_queue_new(struct Logger *logger);
void sync_queue_destroy(struct SyncQueue *queue);

void sync_queue_enqueue(struct SyncQueue *queue, struct SyncQueueEntry *entry);
struct SyncQueueEntry *sync_queue_dequeue(struct SyncQueue *queue);

#endif /* SYNCQUEUE_H */
