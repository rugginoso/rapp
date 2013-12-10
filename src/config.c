#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/queue.h>
#include <yaml.h>
#include "config.h"
#include "logger.h"

struct ConfigValue {
    union {
        long intvalue;
        char *strvalue;
    } value;
    TAILQ_ENTRY(ConfigValue) entries;
};

struct ConfigDescParams {
    ConfigParamType type;
    char *help;
    char *name;
    int range_set;
    long value_min;
    long value_max;
    int multivalued;
    int num_values;
    TAILQ_ENTRY(ConfigDescParams) entries;
    TAILQ_HEAD(ConfigValuesHead, ConfigValue) values;
};

struct ConfigSection {
    char *name;
    int num_params;
    TAILQ_ENTRY(ConfigSection) entries;
    TAILQ_HEAD(ConfigDescParamsHead, ConfigDescParams) desc;
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
    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    struct ConfigSection *sect;
    while (conf->sections.tqh_first != NULL) {
        sect = conf->sections.tqh_first;
        while (sect->desc.tqh_first != NULL) {
            np = sect->desc.tqh_first;
            while (np->values.tqh_first != NULL) {
                cv = np->values.tqh_first;
                TAILQ_REMOVE(&np->values, np->values.tqh_first, entries);
                if (np->type == PARAM_STRING)
                    free(cv->value.strvalue);
                free(cv);
            }
            TAILQ_REMOVE(&sect->desc, sect->desc.tqh_first, entries);
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

struct ConfigDescParams* get_desc_param_in_sect(struct ConfigSection *sect,
                                                const char* name) {
    struct ConfigDescParams *np;
    for (np=sect->desc.tqh_first; np != NULL; np=np->entries.tqe_next) {
        if (strcmp(np->name, name) == 0) {
            return np;
        }
    }
    return NULL;
}

struct ConfigDescParams* get_desc_param(struct Config *conf,
                                        const char *section,
                                        const char *name) {
    struct ConfigSection *sect = get_section(conf, section);
    if (!sect)
        return NULL;
    return get_desc_param_in_sect(sect, name);
}

int config_param_add(struct Config *conf,
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
        TAILQ_INIT(&sect->desc);
        sect->name = strdup(section);
        if (!sect->name) {
            free(sect);
            return 1;
        }
        TAILQ_INSERT_TAIL(&conf->sections, sect, entries);
    }
    size_t size = sizeof(struct ConfigDescParams);
    struct ConfigDescParams *params = (struct ConfigDescParams*) malloc(size);
    if (!params) {
        return 1;
    }
    params->type = type;
    if (help) {
        params->help = strdup(help);
        if (!params->help) {
            free(params);
            return 1;
        }
    }
    params->name = strdup(name);
    if (!params->name) {
        free(params->help);
        free(params);
        return 1;
    }
    TAILQ_INIT(&params->values);
    switch (type) {
        case PARAM_BOOL:
            params->value_min = 0;
            params->value_max = 1;
            break;
        case PARAM_INT:
            params->value_min = LONG_MIN;
            params->value_max = LONG_MAX;
        default:
            params->value_min = params->value_max = 0;
            break;
    }
    params->multivalued = 0;
    params->num_values = 0;
    TAILQ_INSERT_TAIL(&sect->desc, params, entries);
    sect->num_params++;
    return 0;
}

int config_param_set_range_int(struct Config *conf,
                               const char *name,
                               const char *section,
                               long value_min,
                               long value_max) {
    struct ConfigDescParams *np;
    if (!conf || conf->freezed == 1 || value_min > value_max)
        return 1;
    np = get_desc_param(conf, section, name);
    if (!np || np->type != PARAM_INT)
        return 1;
    np->value_min = value_min;
    np->value_max = value_max;
    np->range_set = 1;
    return 0;
}

int config_param_set_multivalued(struct Config *conf,
                                 const char *section,
                                 const char *name,
                                 int flag) {
    if (!conf || conf->freezed == 1 || !section || !name)
        return 1;
    struct ConfigDescParams *np = get_desc_param(conf, section, name);
    if (!np)
        return 1;
    np->multivalued = flag;
    return 0;
}

int config_add_value(struct Config *conf, const char *section,
                     const char *name, const char *value) {
    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    char *endptr;
    long val;
    int *temp;
    if (!conf || !section || !name || !value)
        return 1;
    np = get_desc_param(conf, section, name);
    if (!np)
        return 1;
    if (np->multivalued == 0 && np->num_values > 0) {
        return 1;
    }
    cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
    if (!cv)
        return 1;
    switch (np->type) {
        case PARAM_BOOL:
        case PARAM_INT:
            errno = 0;
            val = strtol(value, &endptr, 10);
            if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                    || (errno != 0 && val == 0)
                    || (np->range_set == 1 && (val < np->value_min) || (val > np->value_max))) {
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
    np->num_values++;
    TAILQ_INSERT_TAIL(&np->values, cv, entries);
    return 0;
}

int config_get_nth_int(struct Config *conf, const char *section,
                       const char *name, int position,
                       long *value) {

    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    int i = 0;
    if (!conf || !section || !name || position < 0)
        return 1;
    np = get_desc_param(conf, section, name);
    if (!np)
        return 1;
    if (np->type != PARAM_INT && np->type != PARAM_BOOL)
        return 1;
    if(np->multivalued == 0 && position != 0)
        return 1;
    for(cv=np->values.tqh_first; cv != NULL && i < position; cv=cv->entries.tqe_next);
    if (!cv)
        return 1;
    *value = cv->value.intvalue;
    return 0;
}

int config_get_nth_bool(struct Config *conf, const char *name, const char *section,
                        int position, int *value) {
    return config_get_nth_int(conf, section, name, position, (long*) value);
}

int config_get_nth_string(struct Config *conf, const char *section,
                          const char *name, int position,
                          const char **value) {

    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    int i = 0;
    if (!conf || !section || !name || position < 0)
        return 1;
    np = get_desc_param(conf, section, name);
    if (!np || np->type != PARAM_STRING || (np->multivalued == 0 && position != 0))
        return 1;
    for(cv=np->values.tqh_first; cv != NULL && i < position; cv=cv->entries.tqe_next);
    if (!cv)
        return 1;
    *value = strdup(cv->value.strvalue);
    return 0;
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
