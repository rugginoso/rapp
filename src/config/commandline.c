/*
 * config/commandline.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <argp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <rapp/rapp_version.h>

#define ARG_INDEX_BASE 128
#define ARG_LOGLEVEL ARG_INDEX_BASE + 0
#define ARG_LOGOUTPUT ARG_INDEX_BASE + 1
#define ARG_LOGNOCOLOR ARG_INDEX_BASE + 2
#define ARG_LOAD ARG_INDEX_BASE + 3
#define ARG_INDEX_OFFSET 150

const char *argp_program_version;
static struct argp_option rappoptions[] = {
  {0, 0, 0, OPTION_DOC, "Logging:", -3},
  {"log-level", ARG_LOGLEVEL, "LOGLEVEL", 0,
    "Log level, one of: {CRITICAL, ERROR, WARN, INFO, DEBUG}"},
  {"log-output", ARG_LOGOUTPUT, "OUTPUT", 0,
    "Log to file, defaults to <stderr>."},
  {"log-nocolor", ARG_LOGNOCOLOR, 0, 0,
    "Disable colors in logs"},
  {0, 0, 0, OPTION_DOC, "Containers:", -2},
  {"load", ARG_LOAD, "PATH", 0, "Load container .so"},
  {0}
};
const int rappoptions_len = sizeof(rappoptions) / sizeof(rappoptions[0]);

void
config_argp_options_destroy(struct Config *conf)
{
  int i = 0;

  for (i = 0; i < conf->num_argp_options; i++) {
    if (conf->options[i].name)
      free((char *)conf->options[i].name);
    if (conf->options[i].arg)
      free((char *)conf->options[i].arg);
    if (conf->options[i].doc)
      free((char *)conf->options[i].doc);
  }
  free(conf->options);
  free(conf->options_map);
}

int
generate_argp_for_section(struct Config *conf, struct ConfigSection *sect,
    int *index, int group)
{
  struct ConfigOption *opt = NULL;
  char *prefix = NULL;
  char *optname = NULL, *metavar = NULL;
  size_t prefix_length = 0, optname_length = 0;
  int i = *index;

  if (strcmp(sect->name, RAPP_CONFIG_SECTION) != 0) {
    // not in core section, we need to prefix options
    // +2: \0 and - as separator from optname
    prefix_length = (strlen(sect->name) + 2) * sizeof(char);
    prefix = (char*) malloc(prefix_length);
    if (!prefix)
      return -1;
    snprintf(prefix, prefix_length, "%s-", sect->name);
  }

  for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
    optname_length = prefix_length + sizeof(char) * strlen(opt->name) + 1;
    optname = (char *) malloc(optname_length);
    if (!optname) {
      return -1;
    }
    if (prefix)
      snprintf(optname, optname_length, "%s%s", prefix, opt->name);
    else
      snprintf(optname, optname_length, "%s", opt->name);

    conf->options[*index].key = (*index) + ARG_INDEX_OFFSET;
    conf->options[*index].name = optname;

    // Do not set arg for bools
    if (opt->type != PARAM_BOOL) {
      if (!opt->metavar) {
        metavar = strdup(opt->name);
        uppercase(metavar);
        conf->options[*index].arg = metavar;
      } else {
        conf->options[*index].arg = strdup(opt->metavar);
      }
    }
    conf->options[*index].group = group;

    if (opt->help)
      conf->options[*index].doc = strdup(opt->help);

    conf->options_map[*index] = opt;

    DEBUG(conf, "Added commandline option '--%s', help: '%s', "
        "arg: %s, index %d, group %d, key %d",
        conf->options[*index].name, conf->options[*index].doc,
        conf->options[*index].arg, *index,
        conf->options[*index].group, conf->options[*index].key);
    (*index)++;
  }

  if (prefix)
    free(prefix);

  return 0;
}

int
config_generate_commandline(struct Config *conf)
{
  size_t argp_option_size = sizeof(struct argp_option);
  struct ConfigSection *s = NULL;
  int index = 0, group = 1, num_options = 0;
  const char *titlebase = "Options for ";
  char *title;
  int title_len, titlebase_len;

  if (!conf)
    return -1;

  titlebase_len = strlen(titlebase);

  // do not support being called twice
  if (conf->options)
    return -1;

  /* Number of options:
   * for each section, 1 for the "title" and 1 for each option
   * one empty entry at the end of the array to mark the end of it
   */
  num_options = 1;
  for(s=conf->sections.tqh_first; s != NULL; s = s->entries.tqe_next) {
    num_options = num_options + s->num_opts + 1;
  }

  // Allocate space for static options as well
  conf->options = calloc(num_options + rappoptions_len,
      (sizeof(struct argp_option)));
  conf->options_map = calloc(num_options + rappoptions_len,
      (sizeof(struct ConfigOption*)));

  // now, add arguments for each sections
  for (s=conf->sections.tqh_first; s != NULL; s=s->entries.tqe_next) {
    // set the section title for help formatting
    conf->options[index].group = group;
    conf->options[index].flags = OPTION_DOC;
    // +4: '\0', '' and :
    title_len = strlen(s->name) + titlebase_len + 4;
    conf->options[index].doc = malloc(title_len * sizeof(char));
    snprintf((char *)conf->options[index].doc, title_len, "%s'%s':",
        titlebase, s->name);
    index++;

    DEBUG(conf, "Creating commandline for section '%s'", s->name);
    if (generate_argp_for_section(conf, s, &index, group) != 0) {
      return -1;
    }

    // increment the group index (for next section, if any)
    group++;
  }

  conf->num_argp_options = index;
  return 0;
}

error_t
parse_commandline_opt(int key, char *arg, struct argp_state *state)
{
  struct Config *conf = state->input;
  struct ConfigOption *opt = NULL;
  int index = 0;

  switch (key) {
    case ARGP_KEY_NO_ARGS:
      return 0;
      break;

    case ARGP_KEY_INIT:
    case ARGP_KEY_FINI:
    case ARGP_KEY_END:
      return 0;
      break;

    case ARGP_KEY_SUCCESS:
      return 0;
      break;

    case ARGP_KEY_ERROR:
      return EINVAL;
      break;
  }
  if (key >= ARG_INDEX_BASE && key < ARG_INDEX_OFFSET) {
    // early options were already parsed
    return 0;
  }
  index = key - ARG_INDEX_OFFSET;

  if (index < 0 || index > conf->num_argp_options) {
    CRITICAL(conf, "key %x maps to index %d which is not in [0. %d]",
        key, index, conf->num_argp_options);
    return EINVAL;
  }

  opt = conf->options_map[index];
  if (!opt) {
    CRITICAL(conf, "Key %x - index %d received but mapping is NULL", key,
        index);
    return EINVAL;
  }

  if (!arg) {
    WARN(conf, "Key %s.%s NULL value", opt->section->name, opt->name);
    return EINVAL;
  }

  // set the value for opt using '*arg'
  DEBUG(conf, "Setting value for %s.%s = %s from commanline",
      opt->section->name, opt->name, arg);

  if (config_add_value_from_string(conf, opt, arg) != 0) {
    CRITICAL(conf, "Cannot set value %s from for %s.%s",
        arg, opt->section->name, opt->name);
    return EINVAL;
  }

  // Do not override the value in case it's present in config
  opt->no_override = 1;
  return 0;
}

int
config_parse_commandline(struct Config *conf, int argc, char* argv[])
{
  int i, index, res;
  struct argp *argp_conf = calloc(1, sizeof(struct argp));
  if (!argp_conf)
    return -1;

  if (config_generate_commandline(conf) != 0)
    return -1;

  // Add early options to print help and usage, and to avoid
  // giving error as not recognized
  for(i = 0; i < rappoptions_len; i++) {
    index = i + conf->num_argp_options;
    conf->options[index].name = rappoptions[i].name;
    conf->options[index].arg = rappoptions[i].arg;
    conf->options[index].key = rappoptions[i].key;
    conf->options[index].group = rappoptions[i].group;
    conf->options[index].doc = rappoptions[i].doc;
  }

  argp_conf->options = conf->options;
  argp_conf->parser = parse_commandline_opt;
  argp_conf->args_doc = "- Quick description for Rapp goes here";
  argp_conf->doc = "Documentation for Rapp goes here";
  res = argp_parse(argp_conf, argc, argv, 0, NULL, conf);
  free(argp_conf);
  return res;
}


error_t
parse_early_opt(int key, char *arg, struct argp_state *state) {
  struct RappArguments *arguments = state->input;
  int lvl;
  switch(key) {
    case ARG_LOGLEVEL:
      if (!arg)
        return EINVAL;
      if (!strcasecmp(arg, "CRITICAL"))
        lvl = LOG_CRITICAL;
      else if (!strcasecmp(arg, "ERROR"))
        lvl = LOG_ERROR;
      else if (!strcasecmp(arg, "WARN"))
        lvl = LOG_WARNING;
      else if (!strcasecmp(arg, "INFO"))
        lvl = LOG_INFO;
      else if (!strcasecmp(arg, "DEBUG"))
        lvl = LOG_DEBUG;
      else
        return 0;
      arguments->loglevel = lvl;
      break;

    case ARG_LOGOUTPUT:
      arguments->logoutput = NULL;
      if (!arg)
        return EINVAL;

      if (strcmp(arg, "-") == 0) {
        arguments->logoutput = stdout;
        return 0;
      }

      arguments->logoutput_is_console = 0;
      arguments->logoutput = fopen(arg, "w");
      if (!arguments->logoutput) {
        logger_panic("Cannot open logfile %s: %s", arg, strerror(errno));
        return EINVAL;
      }

      return 0;
      break;

    case ARG_LOGNOCOLOR:
      arguments->lognocolor = 1;
      break;

    case ARG_LOAD:
      if (!arg)
        return EINVAL;
      arguments->container = strdup(arg);
      break;

    default:
      return 0;
      break;
  }
  return 0;
}

int
config_parse_early_commandline(struct RappArguments *arguments,
                               int argc, char* argv[])
{
  struct argp argp = {rappoptions, parse_early_opt, 0, 0};
  int res;
  argp_program_version = rapp_get_version_full();
  if (argc == 0 || !argv)
    return -1;

  // defaults;
  arguments->loglevel = LOG_INFO;
  arguments->logoutput_is_console = 1;
  arguments->logoutput = stderr;
  arguments->lognocolor = 0;
  arguments->container = NULL;

  res = argp_parse(&argp, argc, argv, ARGP_SILENT, 0,
      arguments);
  return res;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

