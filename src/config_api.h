/*
 * config_api.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef CONFIG_H
#define CONFIG_H
#include "logger.h"

#define RAPP_CONFIG_SECTION "core"
typedef enum {
    PARAM_BOOL,
    PARAM_INT,
    PARAM_STRING
} ConfigParamType;

struct Config;

// PUBLIC
int config_opt_add(struct Config *conf, const char *section,
                   const char *name, ConfigParamType type,
                   const char *help, const char* metavar);
int config_opt_set_range_int(struct Config *conf, const char *section,
                             const char *name, long value_min,
                             long value_max);
int config_opt_set_multivalued(struct Config *conf, const char *section,
                               const char *name, int flag);
int config_opt_set_default_bool(struct Config *conf, const char *section,
                                const char *name, int value);
int config_opt_set_default_int(struct Config *conf, const char *section,
                               const char *name, long value);
int config_opt_set_default_string(struct Config *conf, const char *section,
                                  const char *name, const char *value);

int config_get_nth_bool(struct Config *conf, const char *section,
                        const char *name, int position, int *value);
int config_get_nth_int(struct Config *conf, const char *section,
                       const char *name, int position, long *value);
int config_get_nth_string(struct Config *conf, const char *section,
                          const char *name, int position,
                          char **value);
int config_get_num_values(struct Config *conf, const char *section,
                         const char *name, int *num_values);


#define config_get_bool(CONF, SECTION, NAME, VALUE) \
            config_get_nth_bool(CONF, SECTION, NAME, 0, VALUE)
#define config_get_int(CONF, SECTION, NAME, VALUE) \
            config_get_nth_int(CONF, SECTION, NAME, 0, VALUE)
#define config_get_string(CONF, SECTION, NAME, VALUE) \
            config_get_nth_string(CONF, SECTION, NAME, 0, VALUE)

#endif  /* CONFIG_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
