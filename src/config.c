#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/queue.h>
#include <yaml.h>
#include "config.h"
#include "logger.h"

union Value {
    int intvalue;
    char *strvalue;
};

struct ConfigValue {
    union Value *value;
    LIST_ENTRY(ConfigValue) entries;
};

struct ConfigDescParams {
    ConfigParamType type;
    const char *help;
    const char *name;
    int value_min;
    int value_max;
    int multivalued;
    LIST_ENTRY(ConfigDescParams) entries;
    LIST_HEAD(ConfigValuesHead, ConfigValue) values;
};

struct Config {
    LIST_HEAD(ConfigDescParamsHead, ConfigDescParams) desc;
};


int config_init(struct Config* conf) {
    LIST_INIT(&conf->desc);
    return 0;
}

int config_param_add(struct Config *conf,
                     const char *name,
                     ConfigParamType type,
                     const char *help) {

    int ret;
    size_t size = sizeof(struct ConfigDescParams);
    struct ConfigDescParams *params = (struct ConfigDescParams*) malloc(size);
    params->type = type;
    params->help = strdup(help);
    params->name = strdup(name);
    LIST_INIT(&params->values);
    switch (type) {
        case PARAM_BOOL:
            params->value_min = 0;
            params->value_max = 1;
            break;
        case PARAM_INT:
            params->value_min = INT_MIN;
            params->value_max = INT_MAX;
        default:
            params->value_min = params->value_max = 0;
            break;
    }
    params->multivalued = 0;
    LIST_INSERT_HEAD(&conf->desc, params, entries);
    return 1;
}

int config_param_set_range_int(struct Config *conf,
                               const char *name,
                               int value_min,
                               int value_max) {
    struct ConfigDescParams *np;
    for (np=conf->desc.lh_first; np != NULL; np=np->entries.le_next) {
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
    struct ConfigDescParams *np;
    for (np=conf->desc.lh_first; np != NULL; np=np->entries.le_next) {
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

    for (np=conf->desc.lh_first; np != NULL; np=np->entries.le_next) {
        if (strcmp(np->name, name) == 0) {
            cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
            if (!cv)
                return 1;  // FIXME
            cv->value = (union Value*) malloc(sizeof(union Value));
            if (!cv->value) {
                free(cv);
                return 1;  // FIXME
            }
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
                    cv->value->intvalue = val;
                    LIST_INSERT_HEAD(&np->values, cv, entries);
                    break;
                case PARAM_STRING:
                    cv->value->strvalue = strndup(value, len);
                    LIST_INSERT_HEAD(&np->values, cv, entries);
                    break;
                default:
                    return 1; // FIXME
            }
            return 0;
        }
    }
    return 1;
}

int config_get_nth_int(struct Config *conf, const char *name, int nth, int *value) {

    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    int i=0;
    for (np=conf->desc.lh_first; np != NULL; np = np->entries.le_next) {
        if (strcmp(np->name, name) == 0) {
            assert(np->type == PARAM_INT || np->type == PARAM_BOOL);
            if(np->multivalued == 1 || nth == 0) {
                for(cv=np->values.lh_first; cv != NULL && i < nth; cv=cv->entries.le_next);
                if (cv) {
                    *value = cv->value->intvalue;
                    return 0;
                }
            }
        }
    }
    return 1;
}

int config_get_nth_bool(struct Config *conf, const char *name, int nth, int *value) {
    return config_get_nth_int(conf, name, nth, value);
}

int config_get_nth_string(struct Config *conf, const char *name, int nth,
                          const char **value, size_t *len) {

    struct ConfigDescParams *np;
    struct ConfigValue *cv;
    int i=0;
    for (np=conf->desc.lh_first; np != NULL; np = np->entries.le_next) {
        if (strcmp(np->name, name) == 0) {
            assert(np->type == PARAM_STRING);
            if(np->multivalued == 1 || nth == 0) {
                for(cv=np->values.lh_first; cv != NULL && i < nth; cv=cv->entries.le_next);
                if (cv) {
                    *value = strdup(cv->value->strvalue);
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
