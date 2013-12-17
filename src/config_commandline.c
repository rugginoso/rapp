#include <argp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "config_private.h"

#define ARG_INDEX_OFFSET 128

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
    char *optname = NULL, *metavar = NULL;
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

        conf->options[*index].key = (*index) + ARG_INDEX_OFFSET;
        conf->options[*index].name = optname;

        // Do not set arg for bools
        if (opt->type != PARAM_BOOL) {
            if (!opt->metavar) {
                metavar = strdup(opt->name);
                uppercase(metavar);
                conf->options[*index].arg = metavar;
            } else {
                conf->options[*index].arg = strdup(opt->metavar);
            }
        }
        conf->options[*index].group = group;

        if (opt->help)
            conf->options[*index].doc = strdup(opt->help);

        conf->options_map[*index] = opt;

        DEBUG(conf, "Added commandline option '--%s', help: '%s', "
                "arg: %s, index %d, group %d, key %d",
                conf->options[*index].name, conf->options[*index].doc,
                conf->options[*index].arg, *index,
                conf->options[*index].group, conf->options[*index].key);
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
    int index = 0, group = 1, num_options = 0;
    const char *titlebase = "Options for ";
    char *title;
    int title_len, titlebase_len;

    if (!conf)
        return -1;

    titlebase_len = strlen(titlebase);

    // do not support being called twice
    if (conf->options)
        return -1;

    /* Number of options:
     * for each section, 1 for the "title" and 1 for each option
     * one empty entry at the end of the array to mark the end of it
     */
    num_options = 1;
    for(s=conf->sections.tqh_first; s != NULL; s = s->entries.tqe_next) {
        num_options = num_options + s->num_opts + 1;
    }

    // One empty slot for the empty ending structure
    conf->options = calloc(num_options, (sizeof(struct argp_option)));
    conf->options_map = calloc(num_options, (sizeof(struct ConfigOption*)));

    // now, add arguments for each sections
    for (s=conf->sections.tqh_first; s != NULL; s=s->entries.tqe_next) {
        // set the section title for help formatting
        conf->options[index].group = group;
        conf->options[index].flags = OPTION_DOC;
        // +4: '\0', '' and :
        title_len = strlen(s->name) + titlebase_len + 4;
        conf->options[index].doc = malloc(title_len * sizeof(char));
        snprintf((char *)conf->options[index].doc, title_len, "%s'%s':",
                titlebase, s->name);
        index++;

        DEBUG(conf, "Creating commandline for section '%s'", s->name);
        if (generate_argp_for_section(conf, s, &index, group) != 0) {
            return -1;
        }

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
    int index = 0;

    switch (key) {
        case ARGP_KEY_NO_ARGS:
            return 0;
            break;

        case ARGP_KEY_INIT:
        case ARGP_KEY_FINI:
        case ARGP_KEY_END:
            return 0;
            break;

        case ARGP_KEY_SUCCESS:
            return 0;
            break;

        case ARGP_KEY_ERROR:
            CRITICAL(conf, "Error during argument parsing: %d", key);
            return 0;
            break;
    }

    index = key - ARG_INDEX_OFFSET;

    if (index < 0 || index > conf->num_argp_options) {
        CRITICAL(conf, "key %x maps to index %d which is not in [0. %d]",
                key, index, conf->num_argp_options);
        return ARGP_ERR_UNKNOWN;
    }

    opt = conf->options_map[index];
    if (!opt) {
        CRITICAL(conf, "Key %x - index %d received but mapping is NULL", key,
                index);
        return EINVAL;
    }

    if (!arg) {
        WARN(conf, "Key %s.%s NULL value", opt->section->name, opt->name);
        return EINVAL;
    }

    // set the value for opt using '*arg'
    DEBUG(conf, "Setting value for %s.%s = %s from commanline",
            opt->section->name, opt->name, arg);

    if (config_add_value_from_string(conf, opt, arg) != 0) {
        CRITICAL(conf, "Cannot set value %s from for %s.%s",
                arg, opt->section->name, opt->name);
        return EINVAL;
    }

    // Do not override the value in case it's present in config
    opt->set_from_commandline = 1;
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
