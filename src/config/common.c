/*
 * config/common.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

#include "src/memory.h"
#include "common.h"


/* shamelessly stolen from pyyaml - http://bit.ly/Jhm0C0 */
const char regex_bool_str[] = "^yes|Yes|YES|no|No|NO|true|True|TRUE|false|False|FALSE|on|On|ON|off|Off|OFF|1|0$";
const char regex_bool_true_str[] = "^yes|Yes|YES|true|True|TRUE|on|On|ON|1$";


struct RappConfig *
config_new(struct Logger *logger)
{
  struct RappConfig *conf = NULL;

  assert(logger != NULL);

  if ((conf = memory_create(sizeof(struct RappConfig))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  TAILQ_INIT(&conf->sections);
  conf->num_sections = 0;
  conf->logger = logger;
  conf->options = NULL;

  DEBUG(conf, "Successfully initialized empty config object %p", conf);
  return conf;
}

void
config_destroy(struct RappConfig* conf)
{
  struct ConfigSection *sect = NULL;
  assert(conf != NULL);
  while (conf->sections.tqh_first != NULL) {
    sect = conf->sections.tqh_first;
    TAILQ_REMOVE(&conf->sections, conf->sections.tqh_first, entries);
    config_section_destroy(sect);
  }
  config_argp_options_destroy(conf);
  memory_destroy(conf);
}

static int
opt_add_value_int(struct ConfigOption *opt,
                  long                value)
{
  struct ConfigValue *cv = NULL;

  if (opt->type != PARAM_BOOL && opt->type != PARAM_INT)
    return -1;
  if (opt->multivalued == 0 && opt->num_values > 0)
    return -1;
  if (opt->range_set == 1 && (opt->value_min > value || opt->value_max < value))
    return -1;
  if ((cv = memory_create(sizeof(struct ConfigValue))) == NULL)
    return -1;
  cv->value.intvalue = value;
  opt->num_values++;
  TAILQ_INSERT_TAIL(&opt->values, cv, entries);
  return 0;
}

static int
opt_add_value_string(struct ConfigOption *opt,
                     const char          *value)
{
  struct ConfigValue *cv = NULL;
  if (opt->multivalued == 0 && opt->num_values > 0)
    return -1;
  if (!value || opt->type != PARAM_STRING)
    return -1;
  if ((cv = memory_create(sizeof(struct ConfigValue))) == NULL)
    return -1;
  cv->value.strvalue = memory_strdup(value);
  if (!cv->value.strvalue) {
    memory_destroy(cv);
    return -1;
  }
  opt->num_values++;
  TAILQ_INSERT_TAIL(&opt->values, cv, entries);
  return 0;
}

int
config_add_value_int(struct RappConfig *conf,
                     const char        *section,
                     const char        *name,
                     long              value)
{
  struct ConfigOption *opt = NULL;
  GET_OPTION(opt, conf, section, name);
  if (opt_add_value_int(opt, value) != 0)
    return -1;
  DEBUG(conf, "Added value '%s.%s' = %d", section, name, value);
  return 0;
}

int
config_add_value_string(struct RappConfig *conf,
                        const char        *section,
                        const char        *name,
                        const char        *value)
{
  struct ConfigOption *opt = NULL;
  GET_OPTION(opt, conf, section, name);
  if (opt_add_value_string(opt, value) != 0) {
    return -1;
  }
  DEBUG(conf, "Added value '%s.%s' = '%s'", section, name, value);
  return 0;
}

int
config_add_value_from_string(struct RappConfig   *conf,
                             struct ConfigOption *opt,
                             const char          *value)
{
  long val;
  char *endptr;
  int reti;
  char regerr_msg[100];
  regex_t regex_bool;
  switch(opt->type) {
    case PARAM_STRING:
      opt_add_value_string(opt, value);
      DEBUG(conf, "Added %s.%s = %s", opt->section->name, opt->name, value);
      break;

    case PARAM_BOOL:
      reti = regcomp(&regex_bool, regex_bool_str, REG_EXTENDED);
      if (reti) {
        ERROR(conf, "Cannot compile regex for bool parsing (error: %d)", reti);
        regfree(&regex_bool);
        return -1;
      }
      reti = regexec(&regex_bool, value, 0, NULL, 0);
      if (reti == 0) {  /* MATCH */
        DEBUG(conf, "MATCH %s.%s as boolean : %s", opt->section->name, opt->name, value);
        reti = regcomp(&regex_bool, regex_bool_true_str, REG_EXTENDED);
        val = reti ? 0 : 1;
        opt_add_value_int(opt, val);
        DEBUG(conf, "Added %s.%s = %d", opt->section->name, opt->name, val);
        break;

      } else {
        regerror(reti, &regex_bool, regerr_msg, 100);
        DEBUG(conf, "Bool regex '%s' does not match '%s': %s",
            regex_bool_str, value, regerr_msg);
        ERROR(conf, "Invalid boolean value %s", value);
        regfree(&regex_bool);
        return -1;
      }
      regfree(&regex_bool);
      break;

    case PARAM_INT:
      errno = 0;
      val = strtol(value, &endptr, 10);
      if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
          || (errno != 0 && val == 0)
          || (strlen(endptr) > 0)
          || (val < opt->value_min) || (val > opt->value_max)) {
        ERROR(conf, "Invalid integer %s: found chars '%s', range is [%d%d]",
            value, endptr, opt->value_min, opt->value_max);
        return -1;
      }
      opt_add_value_int(opt, val);
      DEBUG(conf, "Added %s.%s = %d", opt->section->name, opt->name, val);
      break;
  }
  return 0;
}

void
config_option_remove_all_values(struct ConfigOption *opt)
{
  struct ConfigValue *cv = NULL;
  while (opt->values.tqh_first != NULL) {
    cv = opt->values.tqh_first;
    TAILQ_REMOVE(&opt->values, opt->values.tqh_first, entries);
    if (opt->type == PARAM_STRING)
      memory_destroy(cv->value.strvalue);
    memory_destroy(cv);
    opt->num_values--;
  }
}

void
config_option_destroy(struct ConfigOption *opt)
{
  config_option_remove_all_values(opt);
  memory_destroy(opt->name);
  if (opt->default_set && opt->type == PARAM_STRING)
    memory_destroy(opt->default_value.strvalue);
  if (opt->help)
    memory_destroy(opt->help);
  if (opt->metavar)
    memory_destroy(opt->metavar);
  memory_destroy(opt);
}

void
config_section_destroy(struct ConfigSection *sect)
{
  struct ConfigOption *opt = NULL;
  while (sect->options.tqh_first != NULL) {
    opt = sect->options.tqh_first;
    TAILQ_REMOVE(&sect->options, sect->options.tqh_first, entries);
    config_option_destroy(opt);
  }
  memory_destroy(sect->name);
  memory_destroy(sect);
}

const struct ConfigSection*
config_section_find(const struct RappConfig *conf,
                    const char              *section)
{
  const struct ConfigSection *sect = NULL;
  if(!section)
    return NULL;

  for (sect=conf->sections.tqh_first; sect != NULL; sect=sect->entries.tqe_next) {
    if (strcmp(sect->name, section) == 0)
      return sect;
  }
  return NULL;
}

struct ConfigSection*
config_section_get(struct RappConfig *conf,
                   const char        *section)
{
  struct ConfigSection *sect = NULL;
  if(!section)
    return NULL;

  for (sect=conf->sections.tqh_first; sect != NULL; sect=sect->entries.tqe_next) {
    if (strcmp(sect->name, section) == 0)
      return sect;
  }
  return NULL;
}

struct ConfigSection*
config_section_create(struct RappConfig *conf,
                      const char        *name)
{
  struct ConfigSection *sect = memory_create(sizeof(struct ConfigSection));
  if (!sect)
    return NULL;
  TAILQ_INIT(&sect->options);
  sect->name = memory_strdup(name);
  if (!sect->name) {
    memory_destroy(sect);
    return NULL;
  }
  TAILQ_INSERT_TAIL(&conf->sections, sect, entries);
  DEBUG(conf, "Created section '%s'", name);
  return sect;
}

void
uppercase(char *str)
{
  if (!str)
    return;
  while(*str != '\0') {
    *str = toupper((unsigned char) *str);
    str++;
  }
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

