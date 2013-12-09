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
    const char *help;
    const char *name;
    long value_min;
    long value_max;
    int multivalued;
    int num_values;
    TAILQ_ENTRY(ConfigDescParams) entries;
    TAILQ_HEAD(ConfigValuesHead, ConfigValue) values;
};

struct Config {
    TAILQ_HEAD(ConfigDescParamsHead, ConfigDescParams) desc;
    int num_params;
    int freezed;
};


struct Config *config_new(void) {
    struct Config *conf = (struct Config*) malloc(sizeof(struct Config));
    if (!conf)
        return NULL;
    TAILQ_INIT(&conf->desc);
    conf->num_params = 0;
    conf->freezed = 0;
    return conf;
}

void config_destroy(struct Config* conf) {
    assert(conf != NULL);
    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    while (conf->desc.tqh_first != NULL) {
        while (np->values.tqh_first != NULL) {
            cv = np->values.tqh_first;
            TAILQ_REMOVE(&np->values, np->values.tqh_first, entries);
            free(cv);
        }
        np = conf->desc.tqh_first;
        TAILQ_REMOVE(&conf->desc, conf->desc.tqh_first, entries);
        free(np);
    }
    free(conf);
}

int config_param_add(struct Config *conf,
                     const char *name,
                     ConfigParamType type,
                     const char *help) {

    assert(conf != NULL);
    if (conf->freezed == 1)
        return 1;

    size_t size = sizeof(struct ConfigDescParams);
    struct ConfigDescParams *params = (struct ConfigDescParams*) malloc(size);
    params->type = type;
    params->help = strdup(help);
    params->name = strdup(name);
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
    TAILQ_INSERT_TAIL(&conf->desc, params, entries);
    conf->num_params++;
    return 0;
}

int config_param_set_range_int(struct Config *conf,
                               const char *name,
                               long value_min,
                               long value_max) {
    assert(conf != NULL);
    if (conf->freezed == 1)
        return 1;

    struct ConfigDescParams *np;
    for (np=conf->desc.tqh_first; np != NULL; np=np->entries.tqe_next) {
        if (strcmp(np->name, name) == 0) {
            if (np->type != PARAM_INT || value_min >= value_max)
                return 1;
            np->value_min = value_min;
            np->value_max = value_max;
            return 0;
        }
    }
    return 1;
}

int config_param_set_multivalued(struct Config *conf,
                                 const char *name,
                                 int flag) {
    assert(conf != NULL);
    if (conf->freezed == 1)
        return 1;
    struct ConfigDescParams *np;
    for (np=conf->desc.tqh_first; np != NULL; np=np->entries.tqe_next) {
        if (strcmp(np->name, name) == 0) {
            np->multivalued = flag;
            return 0;
        }
    }
    return 1;
}

int config_add_value(struct Config *conf, const char *name, const char *value,
                     size_t len) {
    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    char *endptr;
    long val;
    int *temp;
    if (!value)
        return 1; // FIXME

    for (np=conf->desc.tqh_first; np != NULL; np=np->entries.tqe_next) {
        if (strcmp(np->name, name) == 0) {
            if (np->multivalued == 0 && np->num_values > 0) {
                return 1;
            }
            cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
            if (!cv)
                return 1;  // FIXME
            switch (np->type) {
                case PARAM_BOOL:
                case PARAM_INT:
                    errno = 0;
                    val = strtol(value, &endptr, 10);
                    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                            || (errno != 0 && val == 0)
                            || (val < np->value_min) || (val > np->value_max)) {
                        free(cv);
                        return 1; // FIXME
                    }
                    cv->value.intvalue = val;
                    break;
                case PARAM_STRING:
                    cv->value.strvalue = strndup(value, len);
                    break;
                default:
                    return 1; // FIXME
            }
            np->num_values++;
            TAILQ_INSERT_TAIL(&np->values, cv, entries);
            return 0;
        }
    }
    return 1;
}

int config_get_nth_int(struct Config *conf, const char *name, int position,
                       long *value) {

    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    int i=0;
    for (np=conf->desc.tqh_first; np != NULL; np = np->entries.tqe_next) {
        if (strcmp(np->name, name) == 0) {
            assert(np->type == PARAM_INT || np->type == PARAM_BOOL);
            if(np->multivalued == 1 || position == 0) {
                for(cv=np->values.tqh_first; cv != NULL && i < position;
                    cv=cv->entries.tqe_next);
                if (cv) {
                    *value = cv->value.intvalue;
                    return 0;
                }
            }
        }
    }
    return 1;
}

int config_get_nth_bool(struct Config *conf, const char *name, int position, int *value) {
    return config_get_nth_int(conf, name, position, (long*) value);
}

int config_get_nth_string(struct Config *conf, const char *name, int position,
                          const char **value, size_t *len) {

    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    int i=0;
    for (np=conf->desc.tqh_first; np != NULL; np = np->entries.tqe_next) {
        if (strcmp(np->name, name) == 0) {
            assert(np->type == PARAM_STRING);
            if(np->multivalued == 1 || position == 0) {
                for(cv=np->values.tqh_first; cv != NULL && i < position; cv=cv->entries.tqe_next);
                if (cv) {
                    *value = strdup(cv->value.strvalue);
                    *len = strlen(*value);
                    return 0;
                }
            }
        }
    }
    return 1;
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
