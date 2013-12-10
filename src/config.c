#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/queue.h>
#include <yaml.h>
#include "config.h"
#include "logger.h"

#define GET_OPTION(opt, conf, section, name)                                  \
do {                                                                          \
  if (!(conf) || !(section) || !(name))                                       \
    return 1;                                                                 \
  struct ConfigSection* s = get_section((conf), (section));                   \
  if (!s)                                                                     \
    return 1;                                                                 \
  opt = NULL;                                                                 \
  for (opt=s->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {    \
    if (strcmp(opt->name, (name)) == 0)                                       \
      break;                                                                  \
  }                                                                           \
  if (!opt)                                                                   \
    return 1;                                                                 \
} while (0)

struct ConfigValue {
    union {
        long intvalue;
        char *strvalue;
    } value;
    TAILQ_ENTRY(ConfigValue) entries;
};

struct ConfigOption {
    ConfigParamType type;
    char *help;
    char *name;
    int range_set;
    long value_min;
    long value_max;
    int multivalued;
    int num_values;
    union {
        long intvalue;
        char *strvalue;
    } default_value;
    int default_set;
    TAILQ_ENTRY(ConfigOption) entries;
    TAILQ_HEAD(ConfigValuesHead, ConfigValue) values;
};

struct ConfigSection {
    char *name;
    int num_opts;
    TAILQ_ENTRY(ConfigSection) entries;
    TAILQ_HEAD(ConfigOptionHead, ConfigOption) options;
};

struct Config {
    int freezed;
    int num_sections;
    TAILQ_HEAD(ConfigSectionHead, ConfigSection) sections;
};


struct Config *config_new(void) {
    struct Config *conf = (struct Config*) malloc(sizeof(struct Config));
    if (!conf)
        return NULL;
    TAILQ_INIT(&conf->sections);
    conf->num_sections = 0;
    conf->freezed = 0;
    return conf;
}

void config_destroy(struct Config* conf) {
    assert(conf != NULL);
    struct ConfigOption *np;
    struct ConfigValue *cv;
    struct ConfigSection *sect;
    while (conf->sections.tqh_first != NULL) {
        sect = conf->sections.tqh_first;
        while (sect->options.tqh_first != NULL) {
            np = sect->options.tqh_first;
            while (np->values.tqh_first != NULL) {
                cv = np->values.tqh_first;
                TAILQ_REMOVE(&np->values, np->values.tqh_first, entries);
                if (np->type == PARAM_STRING)
                    free(cv->value.strvalue);
                free(cv);
            }
            TAILQ_REMOVE(&sect->options, sect->options.tqh_first, entries);
            free(np->name);
            free(np->help);
            free(np);
        }
        TAILQ_REMOVE(&conf->sections, conf->sections.tqh_first, entries);
        free(sect->name);
        free(sect);
    }
    free(conf);
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
        sect = (struct ConfigSection*) malloc(sizeof(struct ConfigSection));
        TAILQ_INIT(&sect->options);
        sect->name = strdup(section);
        if (!sect->name) {
            free(sect);
            return 1;
        }
        TAILQ_INSERT_TAIL(&conf->sections, sect, entries);
    }
    size_t size = sizeof(struct ConfigOption);
    struct ConfigOption *opt = (struct ConfigOption*) malloc(size);
    if (!opt) {
        return 1;
    }
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
    return 0;
}

int config_add_value_int(struct Config *conf, const char *section,
                         const char *name, long value) {
    struct ConfigOption *opt;
    struct ConfigValue *cv;
    GET_OPTION(opt, conf, section, name);
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


int config_add_value(struct Config *conf, const char *section,
                     const char *name, const char *value) {
    struct ConfigOption *opt;
    struct ConfigValue *cv;
    char *endptr;
    long val;
    int *temp;
    if (!value)
        return 1;
    GET_OPTION(opt, conf, section, name);
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return 1;
    }
    cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
    if (!cv)
        return 1;
    switch (opt->type) {
        case PARAM_BOOL:
        case PARAM_INT:
            errno = 0;
            val = strtol(value, &endptr, 10);
            if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                    || (errno != 0 && val == 0)
                    || (opt->range_set == 1 && (val < opt->value_min) || (val > opt->value_max))) {
                free(cv);
                return 1;
            }
            cv->value.intvalue = val;
            break;

        case PARAM_STRING:
            cv->value.strvalue = strdup(value);
            break;

        default:
            return 1;
    }
    opt->num_values++;
    TAILQ_INSERT_TAIL(&opt->values, cv, entries);
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
    return 0;
}

int config_opt_set_default_int(struct Config *conf, const char *section,
                               const char *name, long value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    opt->default_value.intvalue = value;
    opt->default_set = 1;
    return 0;
}

int config_opt_set_default_bool(struct Config *conf, const char *section,
                                const char *name, int value) {
    if (value < 0 || value > 1)
        return 1;
    return config_opt_set_default_int(conf, section, name, value);
}


int parse_config(struct Logger *logger) {
    //FIXME
    FILE *fh = fopen("example.yaml", "r");
    yaml_parser_t parser;
    if(!yaml_parser_initialize(&parser)) {
        logger_trace(logger, LOG_CRITICAL, "rapp",
                     "Failed to initialize yaml parser");
    }
}
