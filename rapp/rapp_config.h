/*
 * rapp_config.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_CONFIG_H
#define RAPP_CONFIG_H


typedef enum {
    PARAM_BOOL,
    PARAM_INT,
    PARAM_STRING
} RappConfigParamType;

struct RappConfig;


int rapp_config_opt_add(struct RappConfig *conf, const char *section, const char *name, RappConfigParamType type, const char *help, const char* metavar);
int rapp_config_opt_set_range_int(struct RappConfig *conf, const char *section, const char *name, long value_min, long value_max);
int rapp_config_opt_set_multivalued(struct RappConfig *conf, const char *section, const char *name, int flag);
int rapp_config_opt_set_default_bool(struct RappConfig *conf, const char *section, const char *name, int value);
int rapp_config_opt_set_default_int(struct RappConfig *conf, const char *section, const char *name, long value);
int rapp_config_opt_set_default_string(struct RappConfig *conf, const char *section, const char *name, const char *value);

int rapp_config_get_nth_bool(struct RappConfig *conf, const char *section, const char *name, int position, int *value);
int rapp_config_get_nth_int(struct RappConfig *conf, const char *section, const char *name, int position, long *value);
int rapp_config_get_nth_string(struct RappConfig *conf, const char *section, const char *name, int position, char **value);
int rapp_config_get_num_values(struct RappConfig *conf, const char *section, const char *name, int *num_values);

#define rapp_config_get_bool(CONF, SECTION, NAME, VALUE) \
            rapp_config_get_nth_bool(CONF, SECTION, NAME, 0, VALUE)
#define rapp_config_get_int(CONF, SECTION, NAME, VALUE) \
            rapp_config_get_nth_int(CONF, SECTION, NAME, 0, VALUE)
#define rapp_config_get_string(CONF, SECTION, NAME, VALUE) \
            rapp_config_get_nth_string(CONF, SECTION, NAME, 0, VALUE)

#endif  /* RAPP_CONFIG_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
