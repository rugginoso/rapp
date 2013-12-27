/*
 * config/env.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#define PREFIX "RAPP_"
#define PREFIX_LEN 5

int
get_env_name(char *sect, char *name, char **envname)
{
  size_t len;
  if (strcmp(sect, RAPP_CONFIG_SECTION) != 0) {
    len = (strlen(name) + strlen(sect) + PREFIX_LEN + 2);
    *envname = malloc(sizeof(char) * len);
    if (!*envname)
      return -1;
    snprintf(*envname, len, "%s%s_%s", PREFIX, sect, name);

  } else {
    len = (strlen(name) + PREFIX_LEN + 1);
    *envname = malloc(sizeof(char) * len);
    if (!*envname)
      return -1;
    snprintf(*envname, len, "%s%s", PREFIX, name);
  }

  uppercase(*envname);
  return 0;
}

int
add_value_from_env_list(struct Config *conf, struct ConfigOption *opt,
                        const char *value)
{
 char *token, *saveptr;
 char *val = strdup(value);
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

int
get_env_for_opt(struct Config *conf, struct ConfigOption *opt)
{
  int res;
  char *envname;
  const char *value;
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

  // set no override so values in config files does not
  // override env value;
  if (res == 0)
    opt->no_override = 1;

  free(envname);
  return res;
}

int
config_read_env(struct Config *conf)
{
  struct ConfigSection *s;
  struct ConfigOption *opt;

  if (!conf)
    return -1;

  for(s=conf->sections.tqh_first; s != NULL; s = s->entries.tqe_next) {
    for (opt=s->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
      if (get_env_for_opt(conf, opt) != 0)
        DEBUG(conf, "Error while reading env var for %s.%s", s->name, opt->name);
    }
  }
  return 0;
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
