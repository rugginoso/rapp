#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "memory.h"
#include "logger.h"
#include "syncqueue.h"

TAILQ_HEAD(queue, SyncQueueEntry);

struct SyncQueue {
  struct Logger *logger;

  struct queue head;

  pthread_mutex_t mutex;
  pthread_cond_t condition;
};


struct SyncQueue *
sync_queue_new(struct Logger *logger)
{
  struct SyncQueue *queue = NULL;

  assert(logger != NULL);

  if ((queue = memory_create(sizeof(struct SyncQueue))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  if (pthread_mutex_init(&(queue->mutex), NULL) != 0) {
    /* TODO: report error */
    memory_destroy(queue);
    return NULL;
  }

  if (pthread_cond_init(&(queue->condition), NULL) != 0) {
    /* TODO: report error */
    pthread_mutex_destroy(&(queue->mutex));
    memory_destroy(queue);
    return NULL;
  }

  TAILQ_INIT(&(queue->head));

  queue->logger = logger;

  return queue;
}

void
sync_queue_destroy(struct SyncQueue *queue)
{
  assert(queue != NULL);

  pthread_mutex_destroy(&(queue->mutex));
  pthread_cond_destroy(&(queue->condition));

  memory_destroy(queue);
}

void
sync_queue_enqueue(struct SyncQueue      *queue,
                   struct SyncQueueEntry *entry)
{
  int empty = 0;
  assert(queue != NULL);
  assert(entry != NULL);

  pthread_mutex_lock(&(queue->mutex));

  empty = TAILQ_EMPTY(&(queue->head));

  TAILQ_INSERT_HEAD(&(queue->head), entry, entries);

  if (empty != 0)
    pthread_cond_signal(&(queue->condition));

  pthread_mutex_unlock(&(queue->mutex));assert(queue != NULL);
}

struct SyncQueueEntry *
sync_queue_dequeue(struct SyncQueue *queue)
{
  struct SyncQueueEntry *entry = NULL;

  assert(queue != NULL);

  pthread_mutex_lock(&(queue->mutex));

  while (TAILQ_EMPTY(&(queue->head)) != 0)
    pthread_cond_wait(&(queue->condition), &(queue->mutex));

  entry = TAILQ_LAST(&(queue->head), queue);
  TAILQ_REMOVE(&(queue->head), entry, entries);

  pthread_mutex_unlock(&(queue->mutex));

  return entry;
}

