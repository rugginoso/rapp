/*
 * path.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/param.h>

#include <rapp/rapp.h>

#include "path.h"

int
get_path(char               *path,
         struct HTTPRequest *http_request,
         const char         *document_root)
{
  struct MemoryRange path_range;
  char *p = NULL;

  assert(path != NULL);

  if (http_request_get_url_field_range(http_request, HTTP_URL_FIELD_PATH, &path_range) < 0)
    return -1;

  EXTRACT_MEMORY_RANGE(p, http_request_get_buffer(http_request), path_range);

  strncpy(path, document_root, MAXPATHLEN);
  strncat(path, p, MAXPATHLEN);

  return 0;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

