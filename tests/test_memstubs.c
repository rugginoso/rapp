/*
 * memory.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_memstubs.h"

struct MemConf {
  int enabled;
  int ttl;
  int fail_count;
};

struct MemConf _Conf;


void
memstub_failure_enable(int ttl,
                       int fail_count)
{
  _Conf.enabled = 1;
  _Conf.ttl = ttl;
  _Conf.fail_count = fail_count;
}

void
memstub_failure_disable(void)
{
  _Conf.enabled = 0;
}

static int
can_alloc(void)
{
  int alloc = 0;
  if (_Conf.enabled) {
    _Conf.ttl--;
    if (_Conf.ttl <= 0) {
      _Conf.fail_count--;
      if (_Conf.fail_count < 0) {
        alloc = 1; /* we failed enough */
      } else {
        alloc = 0; /* still expected to fail. */
      }
    } else {
      alloc = 1;
    }
  } else {
    alloc = 1;
  }
#ifdef DEBUG
  fprintf(stderr,
          "can_alloc() -> [%i] enabled=%i ttl=%i fail_count=%i\n",
          alloc, _Conf.enabled, _Conf.ttl, _Conf.fail_count);
#endif
  return alloc;
}

char *
memory_strdup(const char *s)
{
  char *res = NULL;
  if (can_alloc()) {
    res = strdup(s);
  }
  return res;
}

int
memory_asprintf(char **strp, const char *fmt, ...)
{
  int ret = -1;
  if (can_alloc()) {
    va_list args;
    va_start(args, fmt);
    ret = vasprintf(strp, fmt, args);
    va_end(args);
  }
  return ret;
}

void *memory_create(size_t size)
{
  void *ptr = NULL;
  if (can_alloc()) {
    ptr = calloc(1, size);
  }
  return ptr;
}

void *
memory_resize(void  *ptr,
              size_t size)
{
  void *res_ptr = NULL;
  if (can_alloc()) {
    res_ptr = realloc(ptr, size);
  }
  return res_ptr;
}

void
memory_destroy(void *mem)
{
  free(mem);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

