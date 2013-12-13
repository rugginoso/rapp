#include <stdlib.h>
#include <check.h>

/* include the header(s) of the tested code right after */
#include <config_private.h>
#include <logger.h>

#define S "sectname"
#define N "optname"
#define PS PARAM_STRING

int ck_call_res;
#define ck_call(fun, eq, expected, ...)             \
do {                                                \
    ck_call_res = fun(__VA_ARGS__);                 \
    if (eq)                                         \
        ck_assert_int_eq(ck_call_res, expected);    \
    else                                            \
        ck_assert_int_ne(ck_call_res, expected);    \
} while(0)

#define ck_assert_call_ok(fun, ...) ck_call(fun, 1, 0, __VA_ARGS__)
#define ck_assert_call_fail(fun, ...) ck_call(fun, 0, 0, __VA_ARGS__)


struct Config *conf;

void
setup(void)
{
   struct Logger *logger = logger_open_console(LOG_LAST, stderr);
   conf = config_new(logger);
}

void
teardown(void)
{
    config_destroy(conf);
}

struct ConfigOption*
get_test_option(void) {
    struct ConfigSection *sect;
    struct ConfigOption *opt;
    sect = get_section(conf, S);
    for (opt=sect->options.tqh_first; opt != NULL; opt=opt->entries.tqe_next) {
        if (strcmp(opt->name, N) == 0)
            return opt;
    }
    return NULL;
}


START_TEST(test_config_create)
{
    char *value;
    long v;
    ck_assert(conf != NULL);
    // missing required arguments
    ck_assert_call_fail(config_opt_add, NULL, S, N, PS, NULL);
    ck_assert_call_fail(config_opt_add, conf, NULL, N, PS, NULL);
    ck_assert_call_fail(config_opt_add, conf, S, NULL, PS, NULL);

    // setting properties on non-existing option
    ck_assert_call_fail(config_opt_set_range_int, conf, S, N, 0, 1);
    ck_assert_call_fail(config_opt_set_multivalued, conf, S, N, 0);
    ck_assert_call_fail(config_opt_set_default_int, conf, S, N, 1);
    ck_assert_call_fail(config_opt_set_default_string, conf, S, N, "a");
    ck_assert_call_fail(config_opt_set_default_string, conf, S, N, "a");
    // retrieving values from a non-existing option
    ck_assert_call_fail(config_get_nth_bool, conf, S, N, 0, (int *)&v);
    ck_assert_call_fail(config_get_nth_int, conf, S, N, 0, &v);
    ck_assert_call_fail(config_get_nth_string, conf, S, N, 0, &value);
    ck_assert(value == NULL);
    return;
}
END_TEST

START_TEST(test_config_opt_string)
{
    struct ConfigOption *opt;
    char *value;
    int res;
    ck_assert_call_ok(config_opt_add, conf, S, N, PARAM_STRING, "tests");
    opt = get_test_option();
    ck_assert_call_ok(config_opt_set_default_string, conf, S, N, "default");
    ck_assert_str_eq(opt->default_value.strvalue, "default");
    ck_assert_call_ok(config_get_nth_string, conf, S, N, 0, &value);
    ck_assert_str_eq(value, "default");
    free(value);

    ck_assert_call_ok(config_add_value_string, conf, S, N, "value");
    // multivalued is off
    ck_assert_call_fail(config_add_value_string, conf, S, N, "value");
    ck_assert_call_ok(config_get_nth_string, conf, S, N, 0, &value);
    ck_assert_str_eq(value, "value");
    free(value);
    ck_assert_call_ok(config_opt_set_multivalued, conf, S, N, 1);
    ck_assert_call_ok(config_add_value_string, conf, S, N, "value2");
    ck_assert_call_ok(config_get_num_values, conf, S, N, &res);
    ck_assert_int_eq(res, 2);
    ck_assert_call_ok(config_get_nth_string, conf, S, N, 0, &value);
    ck_assert_str_eq(value, "value");
    free(value);
    ck_assert_call_ok(config_get_nth_string, conf, S, N, 1, &value);
    ck_assert_str_eq(value, "value2");
    free(value);

}
END_TEST


static Suite *
config_suite(void)
{
  Suite *s = suite_create("rapp.core.config");
  TCase *tc = tcase_create("rapp.core.config");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_config_create);
  tcase_add_test(tc, test_config_opt_string);
  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = config_suite();
 SRunner *sr = srunner_create(s);
 srunner_set_log(sr, "check_config.log");
 srunner_run_all(sr, CK_NORMAL);
 number_failed = srunner_ntests_failed(sr);
 srunner_free(sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

