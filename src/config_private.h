#ifndef CONFIG_PRIVATE_H
#define CONFIG_PRIVATE_H
#include <sys/queue.h>
#include "config_api.h"

#define LOG(conf, level, fmt, ...) logger_trace(conf->logger, level, "config", fmt, __VA_ARGS__)
#define INFO(conf, fmt, ...) LOG(conf, LOG_INFO, fmt, __VA_ARGS__)
#define WARN(conf, fmt, ...) LOG(conf, LOG_WARNING, fmt, __VA_ARGS__)
#define DEBUG(conf, fmt, ...) LOG(conf, LOG_DEBUG, fmt, __VA_ARGS__)
#define ERROR(conf, fmt, ...) LOG(conf, LOG_ERROR, fmt, __VA_ARGS__)
#define CRITICAL(conf, fmt, ...) LOG(conf, LOG_CRITICAL, fmt, __VA_ARGS__)

#define GET_OPTION_FROM_SECT(opt, conf, sect, optname)                        \
do {                                                                          \
  if (!(conf) || !(sect) || !(optname)) {                                     \
    WARN(conf, "missing or unset parameter %p", conf);                        \
    return 1;                                                                 \
  }                                                                           \
  opt = NULL;                                                                 \
  for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) { \
    if (strcmp(opt->name, (optname)) == 0)                                    \
      break;                                                                  \
  }                                                                           \
  if (!opt) {                                                                 \
    WARN(conf, "No such option %s in section %s", (optname), (sect->name));   \
    return 1;                                                                 \
  }                                                                           \
} while(0)

#define GET_OPTION(opt, conf, section, optname)                               \
do {                                                                          \
  if (!(conf) || !(section) || !(optname)) {                                  \
    WARN(conf, "missing or unset parameter %p", conf);                        \
    return 1;                                                                 \
  }                                                                           \
  struct ConfigSection* s = get_section((conf), (section));                   \
  if (!s) {                                                                   \
    WARN(conf, "No such section: %s", (section));                             \
    return 1;                                                                 \
  }                                                                           \
  GET_OPTION_FROM_SECT(opt, conf, s, optname);                                \
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
    char *metavar;
    int set_from_commandline;
    struct ConfigSection *section;
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
    struct ConfigOption **options_map;
    struct argp_option *options;
    int num_argp_options;
    TAILQ_HEAD(ConfigSectionHead, ConfigSection) sections;
};

struct Config *config_new(struct Logger *logger);
void config_destroy(struct Config *conf);
int config_parse(struct Config *conf, const char* filename);
int config_scan_directory(struct Config *conf, const char* directory,
                          const char *ext);
int config_parse_string(struct Config *conf, const char *source);

struct ConfigSection* get_section(struct Config *conf, const char *section);
struct ConfigSection* section_create(struct Config *conf, const char *name);
void config_section_destroy(struct ConfigSection *sect);

int config_add_value_string(struct Config *conf, const char *section,
                            const char *name, const char* value);
int config_add_value_int(struct Config *conf, const char *section,
                         const char *name, long value);
int config_add_value_from_string(struct Config *conf, struct ConfigOption *opt,
                                 const char *value);
void config_option_remove_all_values(struct ConfigOption *opt);
void config_option_destroy(struct ConfigOption *opt);

void config_argp_options_destroy(struct Config *conf);
int config_generate_commandline(struct Config *conf);
int config_parse_commandline(struct Config *conf, int argc, char *argv[]);
#endif /* CONFIG_PRIVATE_H */
