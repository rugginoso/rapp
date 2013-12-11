#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/queue.h>
#include <yaml.h>
#include "config.h"


#define LOG(conf, level, fmt, ...) logger_trace(conf->logger, level, "config", fmt, __VA_ARGS__)
#define INFO(conf, fmt, ...) LOG(conf, LOG_INFO, fmt, __VA_ARGS__)
#define WARN(conf, fmt, ...) LOG(conf, LOG_WARNING, fmt, __VA_ARGS__)
#define DEBUG(conf, fmt, ...) LOG(conf, LOG_DEBUG, fmt, __VA_ARGS__)
#define ERROR(conf, fmt, ...) LOG(conf, LOG_ERROR, fmt, __VA_ARGS__)
#define CRITICAL(conf, fmt, ...) LOG(conf, LOG_CRITICAL, fmt, __VA_ARGS__)

#define GET_OPTION_FROM_SECT(opt, conf, sect, name)                           \
do {                                                                          \
  if (!(conf) || !(sect) || !(name)) {                                        \
    WARN(conf, "missing or unset parameter %p", conf);                        \
    return 1;                                                                 \
  }                                                                           \
  opt = NULL;                                                                 \
  for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) { \
    if (strcmp(opt->name, (name)) == 0)                                       \
      break;                                                                  \
  }                                                                           \
  if (!opt) {                                                                 \
    WARN(conf, "No such option %s in section %s", (name), (sect->name));      \
    return 1;                                                                 \
  }                                                                           \
} while(0)

#define GET_OPTION(opt, conf, section, name)                                  \
do {                                                                          \
  if (!(conf) || !(section) || !(name)) {                                     \
    WARN(conf, "missing or unset parameter %p", conf);                        \
    return 1;                                                                 \
  }                                                                           \
  struct ConfigSection* s = get_section((conf), (section));                   \
  if (!s) {                                                                   \
    WARN(conf, "No such section: %s", (section));                             \
    return 1;                                                                 \
  }                                                                           \
  GET_OPTION_FROM_SECT(opt, conf, s, name);                                   \
} while(0)

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
    struct Logger *logger;
    TAILQ_HEAD(ConfigSectionHead, ConfigSection) sections;
};


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

int opt_add_value_string(struct ConfigOption *opt, const char *name,
                         const char *value) {
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

int opt_add_value_int(struct ConfigOption *opt, const char *name,
                      long value) {
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
    if (opt_add_value_int(opt, name, value) != 0)
        return 1;
    DEBUG(conf, "Added value '%s.%s' = %d", section, name, value);
    return 0;
}

int config_add_value_string(struct Config *conf, const char *section,
                            const char *name, const char *value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_string(opt, name, value) != 0) {
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

int yaml_parse_init(struct Config *conf, const char *filename,
                    yaml_parser_t *parser) {
    yaml_token_t token;
    // we should get the STREAM_START first
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_STREAM_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: no stream start",
                 filename);
        return 1;
    }

    // read the first token. This must be a start of doc token
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_DOCUMENT_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: no start of doc found",
                 filename);
        return 1;
    }
    // config format is a global mapping of mappings
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: No global mapping found",
                 filename);
        return 1;
    }
    return 0;
}

int yaml_parse_key_value(struct Config *conf,
                         struct ConfigSection *section,
                         yaml_parser_t *parser,
                         int *last) {
    struct ConfigOption *opt;
    char *name, *endptr;
    long val;
    yaml_token_t token;
    yaml_parser_scan(parser, &token);
    *last = 0;
    if (token.type != YAML_KEY_TOKEN) {
        if (token.type == YAML_BLOCK_END_TOKEN) {
            *last = 1;
            return 1;
        }
        ERROR(conf, "Expected key token, got %d", token.type);
        return 1;
    }
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_SCALAR_TOKEN) {
        ERROR(conf, "Expected scalar token, got %d", token.type);
        return 1;
    }

    name = strdup(token.data.scalar.value);
    if (!name)
        return 1;

    GET_OPTION_FROM_SECT(opt, conf, section, name);
    if (!opt) {
        ERROR(conf, "%s.%s not found", section->name, name);
        free(name);
        return 1;
    }

    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_VALUE_TOKEN) {
        ERROR(conf, "Expected value token, got %d", token.type);
        free(name);
        yaml_token_delete(&token);
        return 1;
    }

    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_SCALAR_TOKEN) {
        ERROR(conf, "Expected scalar token, got %d", token.type);
        free(name);
        yaml_token_delete(&token);
        return 1;
    }

    if (opt->multivalued == 0) {
        // TODO: if scalar, we should wipe out values anyway.
        // this is pending support for lists in yaml
        config_option_remove_all_values(opt);
    }

    switch(opt->type) {
        case PARAM_STRING:
            if (opt_add_value_string(opt, name, token.data.scalar.value) != 0) {
                ERROR(conf, "Cannot set value for %s.%s to %s", section->name,
                      name, token.data.scalar.value);
                free(name);
                yaml_token_delete(&token);
                return 1;
            }
            DEBUG(conf, "Added %s.%s = %s", section->name, name, token.data.scalar.value);
            break;
        case PARAM_BOOL:
        case PARAM_INT:
            errno = 0;
            val = strtol(token.data.scalar.value, &endptr, 10);
            if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                    || (errno != 0 && val == 0)
                    || (opt->range_set == 1 && (val < opt->value_min) || (val > opt->value_max))) {
                ERROR(conf, "Invalid integer %s", token.data.scalar.value);
                free(name);
                return 1;
            }
            if (opt_add_value_int(opt, name, val) != 0) {
                ERROR(conf, "Cannot set value for %s.%s to %d", section->name,
                      name, val);
                free(name);
                yaml_token_delete(&token);
                return 1;
            }
            DEBUG(conf, "Added %s.%s = %d", section->name, name, val);
            break;
    }
    yaml_token_delete(&token);
    free(name);
    return 0;
}

int yaml_skip_section(yaml_parser_t *parser) {
    yaml_token_t token;
    int type;
    do {
        yaml_parser_scan(parser, &token);
        type = token.type;
        yaml_token_delete(&token);
    } while(type != YAML_BLOCK_END_TOKEN);
    return 0;
}


int yaml_parse_section(struct Config *conf,
                       const char *filename,
                       const char *sectionname,
                       yaml_parser_t *parser) {

    yaml_token_t token;
    int last;
    struct ConfigSection* section = get_section(conf, sectionname);
    if (!section) {
        INFO(conf, "Skipping unkown section %s", sectionname);
        if (yaml_skip_section(parser) != 0)
            return 1;
        return 0;
    }
    DEBUG(conf, "Inside parse_section for %s", sectionname);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_VALUE_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: expected value, got %d",
                 filename, token.type);
        return 1;
    }
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: expected block, got %d",
                 filename, token.type);
        return 1;
    }
    while(1) {
        if (yaml_parse_key_value(conf, section, parser, &last) != 0) {
            if (last == 1)
                break;
             else
                return 1;
        }
    }
    yaml_token_delete(&token);
    return 0;
}

int config_parse(struct Config *conf, const char* filename) {
    // TODO: multivalue support / list in yaml as values
    // TODO: erase values if section is present twice
    yaml_parser_t parser;
    yaml_token_t token;
    int done, ret = 0;
    char *err;
    FILE *fh = fopen(filename, "r");
    if (!fh) {
        err = strerror(errno);
        CRITICAL(conf, "Cannot open file '%s': %s", filename, err);
        return 1;
    }
    DEBUG(conf, "Parsing file %s", filename);
    memset(&parser, 0, sizeof(parser));
    if(!yaml_parser_initialize(&parser)) {
        CRITICAL(conf, "Cannot initialize YAML parser: %p", parser);
        fclose(fh);
        return 1;
    }
    yaml_parser_set_input_file(&parser, fh);

    if (yaml_parse_init(conf, filename, &parser) != 0) {
        ret = 1;
        goto cleanup;
    }

    while (1) {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_BLOCK_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        if (token.type != YAML_KEY_TOKEN) {
            CRITICAL(conf, "Malformed yaml file %s: expected section name key, got %d",
                     filename, token.type);
            ret = 1;
            goto cleanup;
        }

        yaml_token_delete(&token);
        yaml_parser_scan(&parser, &token);
        if (token.type != YAML_SCALAR_TOKEN) {
            CRITICAL(conf, "Malformed yaml file %s: expected section name, got %d",
                     filename, token.type);
            ret = 1;
            goto cleanup;
        }
        if (yaml_parse_section(conf, filename, token.data.scalar.value,
                               &parser) != 0) {
            ret = 1;
            break;
        }
        yaml_token_delete(&token);
    }
 // TODO: * STREAM END

cleanup:
    yaml_parser_delete(&parser);
    fclose(fh);
    return ret;

}
