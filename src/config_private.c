#include <assert.h>
#include <stdio.h>
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
    config_argp_options_destroy(conf);
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

void config_argp_options_destroy(struct Config *conf) {
    int i=0;
    struct argp_option *ao;

    for (i=0; i < conf->num_argp_options; i++) {
        ao = conf->options[i];
        fprintf(stderr, "Index: %d, ao %p\n", i, ao);
        if (ao->name)
            free((char *)ao->name);
        free(ao);
    }
    free(conf->options);
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
    prefix = NULL;
    prefix_length = 0;

    if (strcmp(sect->name, RAPP_CONFIG_SECTION) != 0) {
        // not in core section, we need to prefix options
        // +2: \0 and - as separator from optname
        prefix_length = (strlen(sect->name) + 2) * sizeof(char);
        prefix = (char*) malloc(prefix_length);
        if (!prefix)
            return -1;
        snprintf(prefix, prefix_length, "%s-", sect->name);
    }

    for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
        conf->options[*index] = calloc(1, sizeof(struct argp_option));
        ao = conf->options[*index];
        optname_length = prefix_length + sizeof(char) * strlen(opt->name) + 1;
        optname = (char *) malloc(optname_length);
        if (!optname) {
            return -1;
        }
        if (prefix)
            snprintf(optname, optname_length, "%s%s", prefix, opt->name);
        else
            snprintf(optname, optname_length, "%s", opt->name);
        ao->name = optname;
        ao->doc = opt->help;
        ao->group = group;
        DEBUG(conf, "Added commandline option '--%s', help: '%s' at index %d",
                ao->name, ao->doc, *index);
        (*index)++;
    }
    if (prefix)
        free(prefix);

    return 0;
}

int config_generate_commandline(struct Config *conf) {
    size_t argp_option_size = sizeof(struct argp_option);
    struct ConfigSection *s;
    struct argp_option *ao;
    int index, group, num_options;
    if (!conf)
        return -1;

    // do not support being called twice
    if (conf->options)
        return -1;

    num_options = 0;
    for(s=conf->sections.tqh_first; s != NULL; s = s->entries.tqe_next) {
        /* We need an empty entry at the start of each section to allow grouping
         * in argp help, except for the first once. Since we need an entry after
         * the last section to mark the end of the array, we just increment after
         * each section and we are ok
         */
        num_options += s->num_opts + 1;
    }
    conf->options = malloc(num_options * (sizeof(struct argp_option*)));

    // now, add arguments for each sections
    index = group = 0;
    for (s=conf->sections.tqh_first; s != NULL; s=s->entries.tqe_next) {
        DEBUG(conf, "Creating commandline for section '%s'", s->name);
        // set the title for this group - if not the first one.
        // the pointer is already on the last allocated structure.
        if (index > 0)
            ao->doc = s->name;

        if (generate_argp_for_section(conf, s, &index, group) != 0) {
            return -1;
        }

        /* Adding an empty line to separate sections. The "doc" field
         * will be set on the next iteration (if any); Otherwise the empty
         * struct is used to mark the end of the array.
         * index is already pointing to the next available slot as it has
         * been incremented in generate_argp_for_section
         */
        conf->options[index] = calloc(1, argp_option_size);
        ao = conf->options[index];
        if (!ao)
            return -1;
        index++;
        // increment the group index (for next section, if any)
        group++;
    }

    conf->num_argp_options = index;
    return 0;
}
