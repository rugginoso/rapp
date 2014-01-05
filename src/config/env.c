/*
 * config/env.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/memory.h"
#include "common.h"


static int
get_env_name(const char *sect,
             const char *name,
             char **envname)
{
  int is_core = (strcmp(sect, RAPP_CONFIG_SECTION) == 0);
  if (memory_asprintf(envname, "RAPP_%s%s%s",
                      (is_core) ?"" :sect,
                      (is_core) ?"" :"_",
                      name) < 0)
    return -1;
  uppercase(*envname);
  return 0;
}

static int
add_value_from_env_list(struct RappConfig   *conf,
                        struct ConfigOption *opt,
                        const char          *value)
{
 char *token = NULL;
 char *saveptr = NULL;
 char *val = memory_strdup(value);
 if (!val)
   return -1;

 DEBUG(conf, "Value: %s", val);

 token = strtok_r(val, ":", &saveptr);
 while(token) {
   config_add_value_from_string(conf, opt, token);
   token = strtok_r(0, ":", &saveptr);
 }

 free(val);
 return 0;
}

static int
get_env_for_opt(struct RappConfig   *conf,
                struct ConfigOption *opt)
{
  int res;
  char *envname = NULL;
  const char *value = NULL;
  /* if the option has been set in commandline, skip it */
  if (opt->no_override == 1)
    return 0;

  if (get_env_name(opt->section->name, opt->name, &envname) != 0)
    return -1;
  DEBUG(conf, "%s.%s -> %s", opt->section->name, opt->name, envname);

  value = getenv(envname);
  if (!value) {
    free(envname);
    return 0;
  }

  DEBUG(conf, "Found in env (%s.%s) %s: %s", opt->section->name, opt->name,
        envname, value);

  if (strstr(value, ":") && opt->multivalued == 1) {
    res = add_value_from_env_list(conf, opt, value);
  } else {
    res = config_add_value_from_string(conf, opt, value);
  }

  /* set no override so values in config files do not override env value */
  if (res == 0)
    opt->no_override = 1;

  free(envname);
  return res;
}

int
config_read_env(struct RappConfig *conf)
{
  struct ConfigSection *s = NULL;
  struct ConfigOption *opt = NULL;

  if (!conf)
    return -1;

  for(s = conf->sections.tqh_first; s != NULL; s = s->entries.tqe_next) {
    for (opt = s->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
      if (get_env_for_opt(conf, opt) != 0)
        DEBUG(conf, "Error while reading env var for %s.%s", s->name, opt->name);
    }
  }
  return 0;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

