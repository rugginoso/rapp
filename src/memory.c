/*
 * memory.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#define _GNU_SOURCE 1
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"


char *
memory_strdup(const char *s)
{
  return strdup(s);
}

int
memory_asprintf(char **strp, const char *fmt, ...)
{
  int ret;
  va_list args;
  va_start(args, fmt);
  ret = vasprintf(strp, fmt, args);
  va_end(args);
  return ret;
}

void *
memory_create(size_t size)
{
  return calloc(1, size);
}

void *
memory_resize(void  *ptr,
              size_t size)
{
  return realloc(ptr, size);
}

void
memory_destroy(void *mem)
{
  free(mem);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

