#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include "config_private.h"

// shamelessly stolen from pyyaml - http://bit.ly/Jhm0C0
const char *regex_bool_str = "^yes|Yes|YES|no|No|NO|true|True|TRUE|false|False|FALSE|on|On|ON|off|Off|OFF$";
const char *regex_bool_true_str = "^yes|Yes|YES|true|True|TRUE|on|On|ON$";

struct Config
*config_new(struct Logger *logger)
{
    struct Config *conf = calloc(1, sizeof(struct Config));
    if (!conf)
        return NULL;
    TAILQ_INIT(&conf->sections);
    conf->num_sections = 0;
    conf->freezed = 0;
    conf->logger = logger;
    conf->options = NULL;
    DEBUG(conf, "Successfully initialized empty config object %p", conf);
    return conf;
}

void
config_destroy(struct Config* conf)
{
    struct ConfigSection *sect = NULL;
    assert(conf != NULL);
    while (conf->sections.tqh_first != NULL) {
        sect = conf->sections.tqh_first;
        TAILQ_REMOVE(&conf->sections, conf->sections.tqh_first, entries);
        config_section_destroy(sect);
    }
    config_argp_options_destroy(conf);
    free(conf);
}

int
opt_add_value_int(struct ConfigOption *opt, long value)
{
    struct ConfigValue *cv = NULL;
    if (opt->type != PARAM_BOOL && opt->type != PARAM_INT) {
        return -1;
    }
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return -1;
    }
    if (opt->range_set == 1 && (opt->value_min > value || opt->value_max < value)) {
            return -1;
    }
    cv = calloc(1, sizeof(struct ConfigValue));
    if (!cv)
        return -1;
    cv->value.intvalue = value;
    opt->num_values++;
    TAILQ_INSERT_TAIL(&opt->values, cv, entries);
    return 0;
}

int
opt_add_value_string(struct ConfigOption *opt, const char *value)
{
    struct ConfigValue *cv = NULL;
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return -1;
    }
    if (!value || opt->type != PARAM_STRING)
        return -1;
    cv = calloc(1, sizeof(struct ConfigValue));
    if (!cv)
        return -1;
    cv->value.strvalue = strdup(value);
    if (!cv->value.strvalue) {
        free(cv);
        return -1;
    }
    opt->num_values++;
    TAILQ_INSERT_TAIL(&opt->values, cv, entries);
    return 0;
}

int
config_add_value_int(struct Config *conf, const char *section,
                     const char *name, long value) {
    struct ConfigOption *opt = NULL;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_int(opt, value) != 0)
        return -1;
    DEBUG(conf, "Added value '%s.%s' = %d", section, name, value);
    return 0;
}

int
config_add_value_string(struct Config *conf, const char *section,
                        const char *name, const char *value) {
    struct ConfigOption *opt = NULL;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_string(opt, value) != 0) {
        return -1;
    }
    DEBUG(conf, "Added value '%s.%s' = '%s'", section, name, value);
    return 0;
}

int
config_add_value_from_string(struct Config *conf, struct ConfigOption *opt,
                             const char* value)
{
    long val;
    char *endptr;
    int reti;
    char regerr_msg[100];
    regex_t regex_bool;
    switch(opt->type) {
        case PARAM_STRING:
            if (opt_add_value_string(opt, value) != 0) {
                ERROR(conf, "Cannot set value for %s.%s to %s", opt->section->name,
                      opt->name, value);
                return -1;
            }
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
            if (reti == 0) {  // MATCH
                DEBUG(conf, "MATCH %s.%s as boolean : %s", opt->section->name, opt->name, value);
                reti = regcomp(&regex_bool, regex_bool_true_str, REG_EXTENDED);
                val = reti ? 0 : 1;
                if (opt_add_value_int(opt, val) != 0) {
                    ERROR(conf, "Cannot set value for %s.%s to %d", opt->section->name,
                          opt->name, val);
                    regfree(&regex_bool);
                    return -1;
                }
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
            if (opt_add_value_int(opt, val) != 0) {
                ERROR(conf, "Cannot set value for %s.%s to %d", opt->section->name,
                      opt->name, val);
                return -1;
            }
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
            free(cv->value.strvalue);
        free(cv);
        opt->num_values--;
    }
}

void
config_option_destroy(struct ConfigOption *opt)
{
    config_option_remove_all_values(opt);
    free(opt->name);
    if (opt->default_set && opt->type == PARAM_STRING)
        free(opt->default_value.strvalue);
    if (opt->help)
        free(opt->help);
    if (opt->metavar)
        free(opt->metavar);
    free(opt);
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
    free(sect->name);
    free(sect);
}

struct ConfigSection*
get_section(struct Config *conf, const char *section)
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
section_create(struct Config *conf, const char *name)
{
    struct ConfigSection *sect = calloc(1, sizeof(struct ConfigSection));
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

