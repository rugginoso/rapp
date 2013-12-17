#include <errno.h>
#include <yaml.h>
#include "config_private.h"

int
yaml_parse_init(struct Config *conf, const char *filename,
                yaml_parser_t *parser)
{
    yaml_token_t token;
    // we should get the STREAM_START first
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_STREAM_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: no stream start",
                 filename);
        return -1;
    }

    // read the first token. This must be a start of doc token
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_DOCUMENT_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: no start of doc found",
                 filename);
        return -1;
    }
    // config format is a global mapping of mappings
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: No global mapping found",
                 filename);
        return -1;
    }
    return 0;
}

int
config_set_value_from_yaml_scalar(struct Config *conf,
                                  struct ConfigOption *opt,
                                  yaml_token_t *token)
{
    return config_add_value_from_string(conf, opt, token->data.scalar.value);
}

int
config_set_value_from_yaml_list(struct Config *conf,
                                struct ConfigOption *opt,
                                yaml_parser_t *parser,
                                yaml_token_t *token)
{
    if (opt->multivalued == 0 ) {
        ERROR(conf, "Option %s.%s does not support multiple values.",
              opt->section->name, opt->name);
        return -1;
    }
    // sequences are BLOCK_ENTRY/SCALAR values
    // read block entry
    yaml_token_delete(token);
    yaml_parser_scan(parser, token);
    while (token->type == YAML_BLOCK_ENTRY_TOKEN ||
           token->type == YAML_SCALAR_TOKEN ||
           token->type == YAML_FLOW_ENTRY_TOKEN) {

        if (token->type != YAML_SCALAR_TOKEN) {
            // read scalar
            yaml_token_delete(token);
            yaml_parser_scan(parser, token);
            if (token->type != YAML_SCALAR_TOKEN) {
                ERROR(conf, "Expected scalar value, got %d", token->type);
                return -1;
            }
        }
        if (config_set_value_from_yaml_scalar(conf, opt, token) != 0) {
            return -1;
        }
        // read block entry
        yaml_token_delete(token);
        yaml_parser_scan(parser, token);
    }
    if (token->type != YAML_BLOCK_END_TOKEN &&
        token->type != YAML_FLOW_SEQUENCE_END_TOKEN) {
        ERROR(conf, "Expected block end token, got %d", token->type);
        return -1;
    }
    return 0;
}


int
yaml_parse_key_value(struct Config *conf,
                     struct ConfigSection *section,
                     yaml_parser_t *parser,
                     int *last)
{
    struct ConfigOption *opt = NULL;
    char *name = NULL;
    yaml_token_t token;

    yaml_parser_scan(parser, &token);
    *last = 0;
    if (token.type != YAML_KEY_TOKEN &&
        token.type != YAML_FLOW_ENTRY_TOKEN) {
        if (token.type == YAML_BLOCK_END_TOKEN ||
            token.type == YAML_FLOW_MAPPING_END_TOKEN) {
            *last = 1;
            return -1;
        }
        ERROR(conf, "Expected key token, got %d", token.type);
        return -1;
    }
    if (token.type == YAML_FLOW_ENTRY_TOKEN) {
        yaml_token_delete(&token);
        yaml_parser_scan(parser, &token);
        if (token.type != YAML_KEY_TOKEN) {
            ERROR(conf, "Expected key token, got %d", token.type);
            return -1;
        }
    }

    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_SCALAR_TOKEN) {
        ERROR(conf, "Expected scalar key name, got %d", token.type);
        return -1;
    }

    name = strdup(token.data.scalar.value);
    if (!name)
        return -1;

    GET_OPTION_FROM_SECT(opt, conf, section, name);
    if (!opt) {
        ERROR(conf, "%s.%s not found", section->name, name);
        free(name);
        return -1;
    }

    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_VALUE_TOKEN) {
        ERROR(conf, "Expected value token, got %d", token.type);
        free(name);
        yaml_token_delete(&token);
        return -1;
    }

    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_SCALAR_TOKEN &&
        token.type != YAML_BLOCK_SEQUENCE_START_TOKEN &&
        token.type != YAML_FLOW_SEQUENCE_START_TOKEN) {
        ERROR(conf, "Expected value as scalar/list got %d", token.type);
        free(name);
        yaml_token_delete(&token);
        return -1;
    }

    // wipe out any previous values, so that this configuration override any
    // previous value read, but does not increase the number of values.
    // This means that for scalar type we overwrite the value and for
    // multivalued we override the whole list.
    // Note that defaults are kept in a separate value so this is not
    // touching those.
    config_option_remove_all_values(opt);

    if (token.type == YAML_BLOCK_SEQUENCE_START_TOKEN ||
        token.type == YAML_FLOW_SEQUENCE_START_TOKEN) {
        if (config_set_value_from_yaml_list(conf, opt, parser, &token) != 0) {
            yaml_token_delete(&token);
            free(name);
            return -1;
        }
    }
    else { // SCALAR
        if (config_set_value_from_yaml_scalar(conf, opt, &token) != 0) {
            yaml_token_delete(&token);
            free(name);
            return -1;
        }
    }

    yaml_token_delete(&token);
    free(name);
    return 0;
}

int
yaml_skip_section(yaml_parser_t *parser)
{
    yaml_token_t token;
    int type;
    do {
        yaml_parser_scan(parser, &token);
        type = token.type;
        yaml_token_delete(&token);
    } while(type != YAML_BLOCK_END_TOKEN);
    return 0;
}


int
yaml_parse_section(struct Config *conf,
                   const char *filename,
                   const char *sectionname,
                   yaml_parser_t *parser)
{

    yaml_token_t token;
    int last;
    struct ConfigSection* section = get_section(conf, sectionname);
    if (!section) {
        INFO(conf, "Skipping unkown section %s", sectionname);
        if (yaml_skip_section(parser) != 0)
            return -1;
        return 0;
    }
    DEBUG(conf, "Inside parse_section for %s", sectionname);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_VALUE_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: expected value, got %d",
                 filename, token.type);
        return -1;
    }
    yaml_token_delete(&token);
    yaml_parser_scan(parser, &token);
    if (token.type != YAML_BLOCK_MAPPING_START_TOKEN &&
        token.type != YAML_FLOW_MAPPING_START_TOKEN) {
        CRITICAL(conf, "Malformed yaml file %s: expected block, got %d",
                 filename, token.type);
        return -1;
    }
    while(1) {
        if (yaml_parse_key_value(conf, section, parser, &last) != 0) {
            if (last == 1)
                break;
             else
                return -1;
        }
    }
    yaml_token_delete(&token);
    return 0;
}

int
config_parse_main(struct Config *conf, yaml_parser_t *parser,
                  const char *sourcename)
{
    yaml_token_t token;
    int done, ret = 0;
    if (yaml_parse_init(conf, sourcename, parser) != 0) {
        ret = -1;
        goto cleanup;
    }

    while (1) {
        yaml_parser_scan(parser, &token);
        if (token.type == YAML_BLOCK_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        if (token.type != YAML_KEY_TOKEN) {
            CRITICAL(conf, "Malformed yaml file %s: expected section name key, got %d",
                     sourcename, token.type);
            ret = -1;
            goto cleanup;
        }

        yaml_token_delete(&token);
        yaml_parser_scan(parser, &token);
        if (token.type != YAML_SCALAR_TOKEN) {
            CRITICAL(conf, "Malformed yaml file %s: expected section name, got %d",
                     sourcename, token.type);
            ret = -1;
            goto cleanup;
        }
        if (yaml_parse_section(conf, sourcename, token.data.scalar.value,
                               parser) != 0) {
            ret = -1;
            break;
        }
        yaml_token_delete(&token);
    }
cleanup:
    yaml_parser_delete(parser);
    return ret;

}

int
config_parse(struct Config *conf, const char* filename)
{
    yaml_parser_t parser;
    char *err;
    int res;
    FILE *fh = fopen(filename, "r");
    if (!fh) {
        err = strerror(errno);
        CRITICAL(conf, "Cannot open file '%s': %s", filename, err);
        return -1;
    }
    DEBUG(conf, "Parsing file %s", filename);
    memset(&parser, 0, sizeof(parser));
    if(!yaml_parser_initialize(&parser)) {
        CRITICAL(conf, "Cannot initialize YAML parser: %p", parser);
        fclose(fh);
        return -1;
    }
    yaml_parser_set_input_file(&parser, fh);
    res = config_parse_main(conf, &parser, filename);
    fclose(fh);
    return res;
}

int
config_parse_string(struct Config *conf, const char *source)
{
    const char *sourcename = "<string>";
    yaml_parser_t parser;

    if (!source || !conf)
        return -1;
    memset(&parser, 0, sizeof(parser));
    if(!yaml_parser_initialize(&parser)) {
        CRITICAL(conf, "Cannot initialize YAML parser: %p", parser);
        return -1;
    }
    yaml_parser_set_input_string(&parser, source, strlen(source));
    return config_parse_main(conf, &parser, sourcename);
}