#ifndef TEST_DLSTUBS_H
#define TEST_DLSTUBS_H

#include <inttypes.h>

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

#endif /* TEST_DLSTUBS_H */

