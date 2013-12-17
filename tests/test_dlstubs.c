#include <string.h>
#include <stdlib.h>

static int errnum = 0;

void *dlopen(const char *filename, int flag)
{
   if (filename && !strcmp(filename, "NULL")) {
     return NULL;
   }
   /* TODO :) */
   return NULL;
}

char *dlerror(void)
{
   return NULL;
}

void *dlsym(void *handle, const char *symbol)
{
   return NULL;
}

int dlclose(void *handle)
{
   /* TODO: what to return if handle == NULL ? */
   /* always succeed. */
   return 0;
}

