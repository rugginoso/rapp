#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "config_private.h"

int config_opt_add(struct Config *conf,
                   const char *section,
                   const char *name,
                   ConfigParamType type,
                   const char *help) {
    if (!conf || conf->freezed == 1 || !section || !name)
        return 1;
    struct ConfigSection *sect = get_section(conf, section);
    if (!sect) {
        sect = section_create(conf, section);
        if (!sect)
            return 1;
    }
    size_t size = sizeof(struct ConfigOption);
    struct ConfigOption *opt = (struct ConfigOption*) calloc(1, size);
    if (!opt) {
        return 1;
    }
    opt->section = sect;
    opt->type = type;
    if (help) {
        opt->help = strdup(help);
        if (!opt->help) {
            free(opt);
            return 1;
        }
    }
    opt->name = strdup(name);
    if (!opt->name) {
        if (opt->help)
            free(opt->help);
        free(opt);
        return 1;
    }
    TAILQ_INIT(&opt->values);
    switch (type) {
        case PARAM_BOOL:
            opt->value_min = 0;
            opt->value_max = 1;
            opt->default_value.intvalue = 0;
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
    opt->multivalued = 0;
    opt->num_values = 0;
    opt->default_set = 0;
    TAILQ_INSERT_TAIL(&sect->options, opt, entries);
    sect->num_opts++;
    DEBUG(conf, "Added parameter '%s.%s' (type %d)", section, name, type);
    return 0;
}

int config_opt_set_range_int(struct Config *conf,
                             const char *section,
                             const char *name,
                             long value_min,
                             long value_max) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (!opt || opt->type != PARAM_INT)
        return 1;
    if (conf->freezed == 1 || value_min > value_max)
        return 1;
    opt->value_min = value_min;
    opt->value_max = value_max;
    opt->range_set = 1;
    DEBUG(conf, "Set range for '%s.%s' to [%d,%d]", section, name, value_min, value_max);
    return 0;
}

int config_opt_set_multivalued(struct Config *conf,
                               const char *section,
                               const char *name,
                               int flag) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (conf->freezed == 1)
        return 1;
    opt->multivalued = flag;
    DEBUG(conf, "Set '%s.%s' as multivalued", section, name);
    return 0;
}

int config_get_nth_int(struct Config *conf, const char *section,
                       const char *name, int position,
                       long *value) {

    struct ConfigOption *opt;
    struct ConfigValue *cv;
    int i = 0;
    if (position < 0)
        return 1;
    GET_OPTION(opt, conf, section, name);
    if (opt->type != PARAM_INT && opt->type != PARAM_BOOL)
        return 1;
    if(opt->multivalued == 0 && position != 0)
        return 1;
    for(cv=opt->values.tqh_first; cv != NULL && i < position; cv=cv->entries.tqe_next);
    if (!cv) {
        if (opt->default_set)
            *value = opt->default_value.intvalue;
        else
            return 1;
    } else {
        *value = cv->value.intvalue;
    }
    return 0;
}

int config_get_nth_bool(struct Config *conf, const char *name, const char *section,
                        int position, int *value) {
    return config_get_nth_int(conf, section, name, position, (long*) value);
}

int config_get_nth_string(struct Config *conf, const char *section,
                          const char *name, int position,
                          const char **value) {

    struct ConfigOption *opt;
    struct ConfigValue *cv;
    int i = 0;
    if (position < 0)
        return 1;
    GET_OPTION(opt, conf, section, name);
    if (opt->type != PARAM_STRING || (opt->multivalued == 0 && position != 0))
        return 1;
    for(cv=opt->values.tqh_first; cv != NULL && i < position; cv=cv->entries.tqe_next);
    if (!cv) {
        if (opt->default_set)
            *value = strdup(opt->default_value.strvalue);
        else
            return 1;
    } else {
        *value = strdup(cv->value.strvalue);
    }
    return 0;
}

int config_get_num_values(struct Config *conf, const char *section,
                          const char *name, int *num_values) {
    if (!conf || !section || !name)
        return 1;

    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    *num_values = opt->num_values;
    return 0;
}

int config_opt_set_default_string(struct Config *conf, const char *section,
                                  const char *name, const char *value) {
    struct ConfigOption *opt;
    if (!value)
        return 1;
    GET_OPTION(opt, conf, section, name);
    opt->default_value.strvalue = strdup(value);
    if (!opt->default_value.strvalue)
        return 1;
    opt->default_set = 1;
    DEBUG(conf, "Set default for '%s.%s' = '%s'", section, name, value);
    return 0;
}

int config_opt_set_default_int(struct Config *conf, const char *section,
                               const char *name, long value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    opt->default_value.intvalue = value;
    opt->default_set = 1;
    DEBUG(conf, "Set default for '%s.%s' = %d", section, name, value);
    return 0;
}

int config_opt_set_default_bool(struct Config *conf, const char *section,
                                const char *name, int value) {
    if (value < 0 || value > 1)
        return 1;
    return config_opt_set_default_int(conf, section, name, value);
}
