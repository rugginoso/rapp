#include <argp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "config_private.h"

void
config_argp_options_destroy(struct Config *conf)
{
    int i = 0;

    for (i = 0; i < conf->num_argp_options; i++) {
        if (conf->options[i].name)
            free((char *)conf->options[i].name);
        if (conf->options[i].arg)
            free((char *)conf->options[i].arg);
        if (conf->options[i].doc)
            free((char *)conf->options[i].doc);
    }
    free(conf->options);
    free(conf->options_map);
}

void
uppercase(char *str)
{
    while(*str != '\0') {
        *str = toupper((unsigned char) *str);
        str++;
    }
}

int
generate_argp_for_section(struct Config *conf, struct ConfigSection *sect,
                          int *index, int group)
{
    struct ConfigOption *opt = NULL;
    char *prefix = NULL;
    char *optname = NULL, *argname = NULL;
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
        optname_length = prefix_length + sizeof(char) * strlen(opt->name) + 1;
        optname = (char *) malloc(optname_length);
        if (!optname) {
            return -1;
        }
        if (prefix)
            snprintf(optname, optname_length, "%s%s", prefix, opt->name);
        else
            snprintf(optname, optname_length, "%s", opt->name);

        // FIXME: 65 is an arbitrary value, it's meant to move key into the
        // range of uppercase ASCII chars, since values near 0 are "taken"
        // by argp constants.
        conf->options[*index].key = (*index) + 65;
        conf->options[*index].group = group;
        conf->options[*index].name = optname;

        // TODO check if opt is bool, if it is do not set ao->arg;
        // For now, we set arg to the uppercase of the option name,
        // without section.
        // TODO: provide an api to set this value as well, like help/doc
        argname = strdup(opt->name);
        uppercase(argname);
        conf->options[*index].arg = argname;

        if (opt->help)
            conf->options[*index].doc = strdup(opt->help);

        conf->options_map[*index] = opt;

        DEBUG(conf, "Added commandline option '--%s', help: '%s', arg: %s, index %d, group %d, key %d",
                conf->options[*index].name, conf->options[*index].doc, conf->options[*index].arg, *index, conf->options[*index].group, conf->options[*index].key);
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
    // One empty slot for the empty ending structure
    conf->options = calloc(num_options + 1, (sizeof(struct argp_option)));
    conf->options_map = calloc(num_options, (sizeof(struct OptionsMap*)));

    // now, add arguments for each sections
    for (s=conf->sections.tqh_first; s != NULL; s=s->entries.tqe_next) {
        DEBUG(conf, "Creating commandline for section '%s'", s->name);
        // set the title for this group - if not the first one.
        // the pointer is already on the last allocated structure.
        if (index > 0 && conf->options[index].doc)
            conf->options[index].doc = s->name;

        if (generate_argp_for_section(conf, s, &index, group) != 0) {
            return -1;
        }

        /* Adding an empty line to separate sections. The "doc" field
         * will be set on the next iteration (if any); Otherwise the empty
         * struct is used to mark the end of the array.
         * index is already pointing to the next available slot as it has
         * been incremented in generate_argp_for_section
         */
        index++;

        // increment the group index (for next section, if any)
        group++;
    }

    // Last entry, 0
    memset(&(conf->options[index]), 0, sizeof(struct argp_option));

    conf->num_argp_options = index;
    return 0;
}

error_t
parse_commandline_opt(int key, char *arg, struct argp_state *state)
{
    struct Config *conf = state->input;
    struct ConfigOption *opt = NULL;
    if (key == ARGP_KEY_NO_ARGS)
        argp_usage(state);

    DEBUG(conf, "key: %d, arg: %s", key, arg);
    if (key == ARGP_KEY_ARG) {
        return 0;
    }

    // FIXME
    key = key - 65;

    if (key > conf->num_argp_options) {
        WARN(conf, "WUT? %d", conf->num_argp_options);
        return 0;
    }

    opt = conf->options_map[key];
    if (!opt) {
        CRITICAL(conf, "Key %d received but mapping is NULL", key);
        return ARGP_ERR_UNKNOWN;
    }

    if (!arg)
        WARN(conf, "Key %s.%s NULL value", opt->section->name, opt->name);

    // set the value for opt using '*arg'
    DEBUG(conf, "Setting value for %s.%s = %s from commanline",
            opt->section->name, opt->name, arg);
    return 0;
}

int
config_parse_commandline(struct Config *conf, int argc, char* argv[])
{
    struct argp *argp_conf = calloc(1, sizeof(struct argp));
    if (!argp_conf)
        return -1;

    if (config_generate_commandline(conf) != 0)
        return -1;

    argp_conf->options = conf->options;
    argp_conf->parser = parse_commandline_opt;
    argp_conf->args_doc = "TODO";
    argp_conf->doc = "Documentation for Rapp goes here";
    argp_parse(argp_conf, argc, argv, 0, NULL, conf);
    return 0;
}
