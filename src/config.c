/*
 * config.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "config_private.h"

int
config_opt_add(struct Config *conf,
               const char *section,
               const char *name,
               ConfigParamType type,
               const char *help,
               const char *metavar)
{
    struct ConfigSection *sect = NULL;
    size_t size = sizeof(struct ConfigOption);
    struct ConfigOption *opt = NULL;

    if (!conf || conf->freezed == 1 || !section || !name)
        return -1;

    sect = get_section(conf, section);
    if (!sect) {
        sect = section_create(conf, section);
        if (!sect)
            return -1;
    }
    opt = (struct ConfigOption*) calloc(1, size);
    if (!opt) {
        return -1;
    }
    opt->section = sect;
    opt->type = type;
    if (help) {
        opt->help = strdup(help);
        if (!opt->help) {
            free(opt);
            return -1;
        }
    }
    opt->name = strdup(name);
    if (!opt->name) {
        if (opt->help)
            free(opt->help);
        free(opt);
        return -1;
    }
    TAILQ_INIT(&opt->values);
    switch (type) {
        case PARAM_BOOL:
            opt->value_min = 0;
            opt->value_max = 1;
            opt->default_value.intvalue = 0;
            opt->range_set = 1;
            break;
        case PARAM_INT:
            opt->value_min = LONG_MIN;
            opt->value_max = LONG_MAX;
            opt->default_value.intvalue = 0;
            break;
        case PARAM_STRING:
            opt->default_value.strvalue = NULL;
            opt->value_min = opt->value_max = 0;
            break;
    }
    if (metavar && type != PARAM_BOOL)
        opt->metavar = strdup(metavar);
    opt->multivalued = 0;
    opt->num_values = 0;
    opt->default_set = 0;
    TAILQ_INSERT_TAIL(&sect->options, opt, entries);
    sect->num_opts++;
    DEBUG(conf, "Added parameter '%s.%s' (type %d)", section, name, type);
    return 0;
}

int
config_opt_set_range_int(struct Config *conf,
                         const char *section,
                         const char *name,
                         long value_min,
                         long value_max)
{
    struct ConfigOption *opt = NULL;
    GET_OPTION(opt, conf, section, name);
    if (!opt || opt->type != PARAM_INT)
        return -1;
    if (conf->freezed == 1 || value_min >= value_max)
        return -1;
    opt->value_min = value_min;
    opt->value_max = value_max;
    opt->range_set = 1;
    DEBUG(conf, "Set range for '%s.%s' to [%d,%d]", section, name, value_min, value_max);
    return 0;
}

int
config_opt_set_multivalued(struct Config *conf,
                           const char *section,
                           const char *name,
                           int flag)
{
    struct ConfigOption *opt = NULL;
    GET_OPTION(opt, conf, section, name);
    if (conf->freezed == 1)
        return -1;
    opt->multivalued = flag;
    DEBUG(conf, "Set '%s.%s' as multivalued", section, name);
    return 0;
}

int
config_get_nth_int(struct Config *conf, const char *section,
                   const char *name, int position,
                   long *value)
{
    struct ConfigOption *opt = NULL;
    struct ConfigValue *cv = NULL;
    int i = 0;
    if (position < 0)
        return -1;
    GET_OPTION(opt, conf, section, name);
    if (opt->type != PARAM_INT && opt->type != PARAM_BOOL)
        return -1;
    if(opt->multivalued == 0 && position != 0)
        return -1;
    if (opt->num_values == 0 && opt->default_set) {
        if (position == 0) {
            *value = opt->default_value.intvalue;
            return 0;
        }
        // position > 0, return error, even if default is set
        return -1;
    }
    if(position >= opt->num_values)
        return -1;
    cv = opt->values.tqh_first;
    while(i < position) {
        cv = cv->entries.tqe_next;
        i++;
    }
    *value = cv->value.intvalue;
    return 0;
}

int
config_get_nth_bool(struct Config *conf, const char *section,
                    const char *name, int position, int *value)
{
    return config_get_nth_int(conf, section, name, position, (long*) value);
}

int
config_get_nth_string(struct Config *conf, const char *section,
                      const char *name, int position,
                      char **value)
{
    struct ConfigOption *opt = NULL;
    struct ConfigValue *cv = NULL;
    int i = 0;
    *value = NULL;
    if (position < 0)
        return -1;
    GET_OPTION(opt, conf, section, name);
    if (opt->type != PARAM_STRING || (opt->multivalued == 0 && position != 0))
        return -1;
    if (opt->num_values == 0 && opt->default_set) {
        if (position == 0) {
            *value = strdup(opt->default_value.strvalue);
            return 0;
        }
        // position > 0, return error, even if default is set
        return -1;
    }
    if(position >= opt->num_values)
        return -1;
    cv = opt->values.tqh_first;
    while(i < position) {
        cv = cv->entries.tqe_next;
        i++;
    }
    *value = strdup(cv->value.strvalue);
    return 0;
}

int
config_get_num_values(struct Config *conf, const char *section,
                      const char *name, int *num_values)
{
    struct ConfigOption *opt = NULL;
    GET_OPTION(opt, conf, section, name);
    *num_values = opt->num_values;
    return 0;
}

int
config_opt_set_default_string(struct Config *conf, const char *section,
                              const char *name, const char *value)
{
    struct ConfigOption *opt = NULL;
    if (!value)
        return -1;
    GET_OPTION(opt, conf, section, name);
    opt->default_value.strvalue = strdup(value);
    if (!opt->default_value.strvalue)
        return -1;
    opt->default_set = 1;
    DEBUG(conf, "Set default for '%s.%s' = '%s'", section, name, value);
    return 0;
}

int
config_opt_set_default_int(struct Config *conf, const char *section,
                           const char *name, long value)
{
    struct ConfigOption *opt = NULL;
    GET_OPTION(opt, conf, section, name);
    opt->default_value.intvalue = value;
    opt->default_set = 1;
    DEBUG(conf, "Set default for '%s.%s' = %d", section, name, value);
    return 0;
}

int
config_opt_set_default_bool(struct Config *conf, const char *section,
                            const char *name, int value)
{
    if (value < 0 || value > 1)
        return -1;
    return config_opt_set_default_int(conf, section, name, value);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

