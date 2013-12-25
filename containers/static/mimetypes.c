/*
 * mimetypes.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <string.h>
#include <assert.h>

/*
 * FIXME: load mime.types file instead of static table.
 */

struct mime {
  const char *ext;
  const char *type;
};

struct mime mime_table[] = {
  {".txt", "text/plain"},
  {".html", "text/html"},
  {".htm", "text/html"},
  {".jpeg", "image/jpeg"},
  {".jpg", "image/jpeg"},
  {".png", "image/png"},
  {".css", "text/css"},
  {".js", "application/javascript"},
  {0, 0}
};

static int
find_extension(const char *path)
{
  int i = 0;

  assert(path != NULL);

  i = strlen(path);

  while(i >= 0) {
    if (path[i] == '.')
      return i;
    i--;
  }

  return -1;
}

const char *
mimetype(const char *path)
{
  int index = -1;
  char *ext = NULL;
  struct mime *m = NULL;

  assert(path != NULL);

  if ((index = find_extension(path)) < 0)
    return "application/octect-stream";

  m = mime_table;
  while(m->ext != NULL) {
    if (strcasecmp(m->ext, &(path[index])) == 0)
      return m->type;
    m++;
  }

  return "application/octect-stream";
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

