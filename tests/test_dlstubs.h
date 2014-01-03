/*
 * test_dlstubs.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef TEST_DLSTUBS_H
#define TEST_DLSTUBS_H

#include <inttypes.h>

#include "rapp/rapp_logger.h"

#define DLSTUB_ERR_NONE   0x00
#define DLSTUB_ERR_DLOPEN 0x01
#define DLSTUB_ERR_DLSYM  0x02
#define DLSTUB_ERR_PLUGIN 0x04

#define DLSTUB_MAX_SYMS   32
/* yes, arbitrary */

struct Symbol {
  char *name;
  uint32_t flags;
};

void dlstub_setup(uint32_t flags, struct Symbol *syms);
int dlstub_get_invoke_count(const char *sym);
void dlstub_debug(const char *tag);

struct Logger *dlstub_logger_get(void);

#endif /* TEST_DLSTUBS_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

