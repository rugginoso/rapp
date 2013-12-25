/*
 * errors.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <errno.h>

#include "errors.h"

unsigned
errno_to_http_code()
{
  unsigned http_code = 0;

  switch(errno) {
  case ENOTDIR:
  case EACCES:
    http_code = 403;
    break;
  case ENOENT:
    http_code = 404;
    break;
  default:
    http_code = 500;
  }

  return http_code;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

