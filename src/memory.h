/*
 * memory.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef MEMORY_H
#define MEMORY_H


char *memory_strdup(const char *s);

int memory_asprintf(char **strp, const char *fmt, ...);

void *memory_create(size_t size);

void *memory_resize(void *ptr, size_t size);

void memory_destroy(void *mem);

#endif  /* MEMORY_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

