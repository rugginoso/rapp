#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <inttypes.h>

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

int ck_call_res;
#define ck_call(fun, eq, expected, ...)             \
do {                                                \
    ck_call_res = fun(__VA_ARGS__);                 \
    if (eq == 1)                                    \
        ck_assert_int_eq(ck_call_res, expected);    \
    else                                            \
        ck_assert_int_ne(ck_call_res, expected);    \
} while(0)

#define ck_assert_call_ok(fun, ...) ck_call(fun, 1, 0, __VA_ARGS__)
#define ck_assert_call_fail(fun, ...) ck_call(fun, 0, 0, __VA_ARGS__)

int connect_to(const char *host, uint16_t port);

int listen_to(const char *host, uint16_t port);

#endif /* TEST_UTILS_H */
