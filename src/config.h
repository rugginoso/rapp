#ifndef CONFIG_H
#define CONFIG_H

typedef enum {
    PARAM_BOOL,
    PARAM_INT,
    PARAM_STRING
} ConfigParamType;

struct Config;

// used by core
int config_init(struct Config *conf);
int config_add_value(struct Config *conf, const char *name,
                     const char* value, size_t len);

// these functions and macros can be accessed by plugins
int config_param_add(struct Config *conf, const char *name,
                     ConfigParamType type, const char *help);
int config_param_set_range_int(struct Config *conf, const char *name,
                               int value_min, int value_max);
int config_param_set_multivalued(struct Config *conf, const char *name,
                                 int flag);

int config_get_nth_bool(struct Config *conf, const char *name, int nth, int *value);
int config_get_nth_int(struct Config *conf, const char *name, int nth, int *value);
int config_get_nth_string(struct Config *conf, const char *name, int nth,
                          const char **value, size_t *len);

#define config_get_bool(CONF, NAME, VALUE) \
            config_get_nth_bool(CONF, NAME, 0, VALUE)
#define config_get_int(CONF, NAME, VALUE) \
            config_get_nth_int(CONF, NAME, 0, VALUE)
#define config_get_string(CONF, NAME, VALUE) \
            config_get_nth_string(CONF, NAME, 0, VALUE)

#endif  /* CONFIG_H */
