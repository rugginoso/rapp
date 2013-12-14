#include <assert.h>
#include <stdlib.h>
#include <string.h>
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
    conf->options = NULL;
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

int opt_add_value_int(struct ConfigOption *opt, long value) {
    struct ConfigValue *cv;
    if (opt->type != PARAM_BOOL && opt->type != PARAM_INT) {
        return -1;
    }
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return -1;
    }
    if (opt->range_set == 1 && (opt->value_min > value || opt->value_max < value)) {
            return -1;
    }
    cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
    if (!cv)
        return -1;
    cv->value.intvalue = value;
    opt->num_values++;
    TAILQ_INSERT_TAIL(&opt->values, cv, entries);
    return 0;
}

int opt_add_value_string(struct ConfigOption *opt, const char *value) {
    struct ConfigValue *cv;
    if (opt->multivalued == 0 && opt->num_values > 0) {
        return -1;
    }
    if (!value || opt->type != PARAM_STRING)
        return -1;
    cv = (struct ConfigValue*) malloc(sizeof(struct ConfigValue));
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

int config_add_value_int(struct Config *conf, const char *section,
                         const char *name, long value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_int(opt, value) != 0)
        return -1;
    DEBUG(conf, "Added value '%s.%s' = %d", section, name, value);
    return 0;
}

int config_add_value_string(struct Config *conf, const char *section,
                            const char *name, const char *value) {
    struct ConfigOption *opt;
    GET_OPTION(opt, conf, section, name);
    if (opt_add_value_string(opt, value) != 0) {
        return -1;
    }
    DEBUG(conf, "Added value '%s.%s' = '%s'", section, name, value);
    return 0;
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

// Commandline
//
int generate_argp_for_section(struct Config *conf, struct ConfigSection *sect, int *index, int group)
{
    struct ConfigOption *opt;
    struct argp_option *ao;
    char *prefix, *optname;
    size_t prefix_length, optname_length;
    int i = *index;
    if (strcmp(sect->name, RAPP_CONFIG_SECTION) != 0) {
        // +2: \0 and - as separator from optname
        prefix_length = (strlen(sect->name) + 2) * sizeof(char);
        prefix = (char*) malloc(prefix_length);
        if (!prefix)
            return -1;
        snprintf(prefix, prefix_length, "%s-", sect->name);
    } else {
        prefix = NULL;
        prefix_length = 0;
    }

    for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
        ao = &(*conf->options)[*index];
        memset(ao, 0, sizeof(struct argp_option));
        optname_length = prefix_length + sizeof(char) * strlen(opt->name) + 1;
        optname = (char *) malloc(optname_length);
        if (!optname) {
            goto cleanup_on_error;
        }
        if (prefix)
            snprintf(optname, optname_length, "%s%s", prefix, opt->name);
        else
            snprintf(optname, optname_length, "%s", opt->name);
        ao->name = optname;
        if (opt->help) {
            ao->doc = strdup(opt->help);
            if (!ao->doc) {
                free(optname);
                goto cleanup_on_error;
            }
        }
        ao->group = group;
        DEBUG(conf, "Added commandline option '--%s', help: '%s'", ao->name, ao->doc);
        *index++;
    }

    return 0;

cleanup_on_error:
    for (i=0; i < *index; i++) {
        ao = &(*conf->options)[i];
        free((char *)ao->name);
        if (ao->doc)
            free((char *)ao->doc);
    }
    if (prefix)
        free(prefix);
    return -1;
}

int config_generate_commandline(struct Config *conf) {
    int num_total_options;
    int options_array_size;
    size_t argp_option_size = sizeof(struct argp_option);
    struct ConfigSection *s;
    int index, group;
    if (!conf)
        return -1;

    // do not support being called twice
    if (conf->options)
        return -1;

    /* ok here is the deal. For each section, for each option we have
     * to create the array: so first "discover" the number of total options */
    num_total_options = 0;
    for (s=conf->sections.tqh_first; s != NULL; s=s->entries.tqe_next)
        num_total_options += s->num_opts;

    DEBUG(conf, "Creating commandline for %d configured options",
            num_total_options);

    /* array must be terminated by an entry with all zeros. Hence the +1
     * Plus, to allow grouping of args in the help output, we add an empty
     * entry for each section, except core. So num_sections - 1. Hence no
     * more +1 :) */
    options_array_size =  num_total_options + conf->num_sections;
    conf->options = malloc(
            argp_option_size * options_array_size);
    if (!conf->options)
        return -1;

    // now, add arguments for each sections
    index = group = 0;
    for (s=conf->sections.tqh_first; s != NULL; s=s->entries.tqe_next) {
        DEBUG(conf, "Creating commandline for section '%s'", s->name);
        // set the title for this group - if not the first one
        if (index > 0)
            (*conf->options)[index].doc = strdup(s->name);
        if (generate_argp_for_section(conf, s, &index, group) != 0) {
            free(conf->options);
            return -1;
        }
        // add the empty entry to separate groups/sections
        memset(&(*conf->options)[index], 0, argp_option_size);
        index++;
        // increment the group index (for next section, if any)
        group++;
    }
    // add the last entry, empty
    memset(&(*conf->options)[index], 0, argp_option_size);

    return 0;
}
