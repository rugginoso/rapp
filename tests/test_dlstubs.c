
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "rapp/rapp_version.h"

#include "test_dlstubs.h"

struct FakeHandle {
    uint32_t flags;
    struct Symbol syms[DLSTUB_MAX_SYMS];
    int invoke_count[DLSTUB_MAX_SYMS];
    int lookup_count[DLSTUB_MAX_SYMS];
    int errors;
    void *cookie;
};

static struct FakeHandle dummy;

void
dlstub_debug(const char *tag)
{
  int i = 0;
  assert(tag != NULL);
  fprintf(stderr, "[%s] errors=%i\n", tag, dummy.errors);
  for (i = 0; i < DLSTUB_MAX_SYMS && dummy.syms[i].name != NULL; i++) {
    fprintf(stderr, "[%s] #%02i %s: 0x%X (%i)\n",
            tag, i, dummy.syms[i].name, dummy.syms[i].flags,
            dummy.invoke_count[i]);
  }
}

static int
is_enabled(const char *sym, uint32_t flag)
{
  int i = 0;
  for (i = 0; i < DLSTUB_MAX_SYMS && dummy.syms[i].name != NULL; i++) {
    if (!(dummy.syms[i].flags & flag)
     && !strcmp(dummy.syms[i].name, sym)) {
      return 1;
    }
  }
  return 0;
}

static void
mark_lookedup(const char *sym)
{
  int i = 0;
  assert(sym != NULL);
  for (i = 0; i < DLSTUB_MAX_SYMS && dummy.syms[i].name != NULL; i++) {
    if (!strcmp(dummy.syms[i].name, sym)) {
      dummy.lookup_count[i]++;
      break;
    }
  }
}


static void
mark_invoked(const char *sym)
{
  int i = 0;
  assert(sym != NULL);
  for (i = 0; i < DLSTUB_MAX_SYMS && dummy.syms[i].name != NULL; i++) {
    if (!strcmp(dummy.syms[i].name, sym)) {
      dummy.invoke_count[i]++;
      break;
    }
  }
}

static int
dummy_get_abi_version(void)
{
  mark_invoked("rapp_get_abi_version");
  if (is_enabled("rapp_get_abi_version", DLSTUB_ERR_PLUGIN)) {
    return ABI_VERSION;
  }
  return 0;
}

static void *
dummy_create(void *cookie, int ac, char **av, int *err)
{
  mark_invoked("rapp_create");
  if (is_enabled("rapp_create", DLSTUB_ERR_PLUGIN)) {
    dummy.cookie = cookie;
    return &dummy;
  }
  return NULL;
}

static void *
lookup_sym(const char *sym)
{
  assert(sym);
  if (!strcmp(sym, "rapp_get_abi_version")) {
    return &dummy_get_abi_version;
  } else if (!strcmp(sym, "rapp_create")) {
    return &dummy_create;
  }
  return NULL;
}

void *
dlsym(void       *handle,
      const char *symbol)
{
  int i = 0;
  assert(handle);
  assert(symbol);
  if (is_enabled(symbol, DLSTUB_ERR_DLSYM)) {
    mark_lookedup(symbol);
    return lookup_sym(symbol);
  } else {
    dummy.errors++;
  }
  return NULL; /* fallback */
}

int
dlclose(void *handle)
{
   /* TODO: what to return if handle == NULL ? */
   /* always succeed. */
   return 0;
}

void *
dlopen(const char *filename,
       int         flag)
{
   assert(filename != NULL);
   return (dummy.flags & DLSTUB_ERR_DLOPEN) ?NULL :&dummy;
}

char *
dlerror(void)
{
   return (dummy.errors) ?"dlstub error" :NULL;
}

void
dlstub_setup(uint32_t flags, struct Symbol *syms)
{
  int i = 0;
  assert(syms != NULL);
  memset(&dummy, 0, sizeof(dummy));
  dummy.flags = flags;
  for (i = 0; i < DLSTUB_MAX_SYMS && syms[i].name != NULL; i++) {
    free(dummy.syms[i].name);
    dummy.syms[i].name = strdup(syms[i].name);
    dummy.syms[i].flags = syms[i].flags;
  }
}

int
dlstub_get_invoke_count(const char *sym)
{
  int i = 0;
  assert(sym != NULL);
  for (i = 0; i < DLSTUB_MAX_SYMS && dummy.syms[i].name != NULL; i++) {
    if (!strcmp(dummy.syms[i].name, sym)) {
      return dummy.invoke_count[i];
    }
  }
  return 0;
}
int
dlstub_get_lookup_count(const char *sym)
{
  int i = 0;
  assert(sym != NULL);
  for (i = 0; i < DLSTUB_MAX_SYMS && dummy.syms[i].name != NULL; i++) {
    if (!strcmp(dummy.syms[i].name, sym)) {
      return dummy.lookup_count[i];
    }
  }
  return 0;
}

