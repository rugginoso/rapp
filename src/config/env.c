/*
 * config/env.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#include "common.h"

int
config_read_env(struct Config *conf, char *envp[])
{
  if (!conf || !envp)
    return -1;
  return 0;
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
