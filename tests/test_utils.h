#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <inttypes.h>

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

int connect_to(const char *host, uint16_t port);

int listen_to(const char *host, uint16_t port);

#endif /* TEST_UTILS_H */
