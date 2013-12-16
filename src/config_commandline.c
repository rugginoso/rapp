#include <argp.h>
#include <string.h>
#include <stdlib.h>
#include "config_private.h"

void
config_argp_options_destroy(struct Config *conf)
{
    int i=0;
    struct argp_option *ao = NULL;

    for (i=0; i < conf->num_argp_options; i++) {
        ao = conf->options[i];
        if (ao->name)
            free((char *)ao->name);
        free(ao);

    }
    free(conf->options);
    free(conf->options_map);
}

int
generate_argp_for_section(struct Config *conf, struct ConfigSection *sect,
                          int *index, int group)
{
    struct ConfigOption *opt = NULL;
    struct argp_option *ao = NULL;
    char *prefix = NULL;
    char *optname = NULL;
    size_t prefix_length = 0, optname_length = 0;
    int i = *index;

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
        if (!ao)
            return -1;
        optname_length = prefix_length + sizeof(char) * strlen(opt->name) + 1;
        optname = (char *) malloc(optname_length);
        if (!optname) {
            return -1;
        }
        if (prefix)
            snprintf(optname, optname_length, "%s%s", prefix, opt->name);
        else
            snprintf(optname, optname_length, "%s", opt->name);

        ao->key = *index;
        ao->name = optname;
        ao->doc = opt->help;
        ao->group = group;
        conf->options_map[*index] = opt;

        DEBUG(conf, "Added commandline option '--%s', help: '%s' at index %d",
                ao->name, ao->doc, *index);
        (*index)++;
    }
    if (prefix)
        free(prefix);

    return 0;
}

int
config_generate_commandline(struct Config *conf)
{
    size_t argp_option_size = sizeof(struct argp_option);
    struct ConfigSection *s = NULL;
    struct argp_option *ao = NULL;
    int index = 0, group = 0, num_options = 0;
    if (!conf)
        return -1;

    // do not support being called twice
    if (conf->options)
        return -1;

    for(s=conf->sections.tqh_first; s != NULL; s = s->entries.tqe_next) {
        /* We need an empty entry at the start of each section to allow grouping
         * in argp help, except for the first once. Since we need an entry after
         * the last section to mark the end of the array, we just increment after
         * each section and we are ok
         */
        num_options += s->num_opts + 1;
    }
    conf->options = calloc(num_options, (sizeof(struct argp_option*)));
    conf->options_map = calloc(num_options, (sizeof(struct OptionsMap*)));

    // now, add arguments for each sections
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

error_t
parse_commandline_opt(int key, char *arg, struct argp_state *state)
{
    struct Config *conf = state->input;
    struct ConfigOption *opt = NULL;
    if (key == ARGP_KEY_NO_ARGS) {
        argp_usage(state);
        return -1;
    }
    if (key < 0 || key > conf->num_argp_options)
        return ARGP_ERR_UNKNOWN;

    opt = conf->options_map[key];
    if (!opt) {
        CRITICAL(conf, "Key %d received but mapping is NULL", key);
        return ARGP_ERR_UNKNOWN;
    }
    // set the value for opt using '*arg'
    DEBUG(conf, "Setting value for %s.%s = %s from commanline",
            opt->section->name, opt->name, arg);
    return 0;
}

int
config_parse_commandline(struct Config *conf, int argc, char* argv[])
{
    const char *doc = NULL;
    struct argp *argp_conf = calloc(1, sizeof(struct argp));
    if (!argp_conf)
        return -1;
    doc = strdup("Documentation for Rapp goes here");
    if (!doc)
        return -1;
    if (config_generate_commandline(conf) != 0)
        return -1;
    argp_conf->options = *conf->options;
    argp_conf->parser = parse_commandline_opt;
    argp_conf->args_doc = "TODO";
    argp_conf->doc = doc;
    argp_parse (argp_conf, argc, argv, 0, 0, conf);
    return 0;
}