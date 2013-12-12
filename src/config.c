#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "config_private.h"


struct Config *config_new(struct Logger *logger) {
    struct Config *conf = (struct Config*) malloc(sizeof(struct Config));
    if (!conf)
        return NULL;
    TAILQ_INIT(&conf->sections);
    conf->num_sections = 0;
    conf->freezed = 0;
    conf->logger = logger;
    DEBUG(conf, "Successfully initialized empty config object %p", conf);
    return conf;
}

void config_destroy(struct Config* conf) {
    assert(conf != NULL);
    struct ConfigSection *sect;
    while (conf->sections.tqh_first != NULL) {
        sect = conf->sections.tqh_first;
        TAILQ_REMOVE(&conf->sections, conf->sections.tqh_first, entries);
        config_section_destroy(sect);
    }
    free(conf);
}

int config_opt_add(struct Config *conf,
                   const char *section,
                   const char *name,
                   ConfigParamType type,
                   const char *help) {
    assert(conf != NULL);
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

int opt_add_value_string(struct ConfigOption *opt, const char *value) {
    struct ConfigValue *cv;
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return 1;
    }
    if (!value || opt->type != PARAM_STRING)
        return 1;
    cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
    if (!cv)
        return 1;
    cv->value.strvalue = strdup(value);
    if (!cv->value.strvalue) {
        free(cv);
        return 1;
    }
    opt->num_values++;
    TAILQ_INSERT_TAIL(&opt->values, cv, entries);
    return 0;
}

int opt_add_value_int(struct ConfigOption *opt, long value) {
    struct ConfigValue *cv;
    if (opt->type != PARAM_BOOL && opt->type != PARAM_INT) {
        return 1;
    }
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return 1;
    }
    if (opt->range_set == 1 && (opt->value_min > value || opt->value_max < value)) {
            return 1;
    }
    cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
    if (!cv)
        return 1;
    cv->value.intvalue = value;
    opt->num_values++;
    TAILQ_INSERT_TAIL(&opt->values, cv, entries);
    return 0;
}

int config_add_value_int(struct Config *conf, const char *section,
                         const char *name, long value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_int(opt, value) != 0)
        return 1;
    DEBUG(conf, "Added value '%s.%s' = %d", section, name, value);
    return 0;
}

int config_add_value_string(struct Config *conf, const char *section,
                            const char *name, const char *value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_string(opt, value) != 0) {
        return 1;
    }
    DEBUG(conf, "Added value '%s.%s' = '%s'", section, name, value);
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

void config_option_remove_all_values(struct ConfigOption *opt) {
    struct ConfigValue *cv;
    while (opt->values.tqh_first != NULL) {
        cv = opt->values.tqh_first;
        TAILQ_REMOVE(&opt->values, opt->values.tqh_first, entries);
        if (opt->type == PARAM_STRING)
            free(cv->value.strvalue);
        free(cv);
        opt->num_values--;
    }
}

void config_option_destroy(struct ConfigOption *opt) {
    config_option_remove_all_values(opt);
    free(opt->name);
    if (opt->default_set && opt->type == PARAM_STRING)
        free(opt->default_value.strvalue);
    if (opt->help)
        free(opt->help);
    free(opt);
}

void config_section_destroy(struct ConfigSection *sect) {
    struct ConfigOption *opt;
    while (sect->options.tqh_first != NULL) {
        opt = sect->options.tqh_first;
        TAILQ_REMOVE(&sect->options, sect->options.tqh_first, entries);
        config_option_destroy(opt);
    }
    free(sect->name);
    free(sect);
}

struct ConfigSection* get_section(struct Config *conf, const char *section) {
    struct ConfigSection *sect;
    if(!section)
        return NULL;

    for (sect=conf->sections.tqh_first; sect != NULL; sect=sect->entries.tqe_next) {
        if (strcmp(sect->name, section) == 0)
            return sect;
    }
    return NULL;
}

struct ConfigSection* section_create(struct Config *conf, const char *name) {
    struct ConfigSection *sect;
    sect = (struct ConfigSection*) malloc(sizeof(struct ConfigSection));
    if (!sect)
        return NULL;
    TAILQ_INIT(&sect->options);
    sect->name = strdup(name);
    if (!sect->name) {
        free(sect);
        return NULL;
    }
    TAILQ_INSERT_TAIL(&conf->sections, sect, entries);
    DEBUG(conf, "Created section '%s'", name);
    return sect;
}
