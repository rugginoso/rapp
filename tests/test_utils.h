/*
 * test_utils.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <inttypes.h>

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

int connect_to(const char *host, uint16_t port);

int listen_to(const char *host, uint16_t port);

#endif /* TEST_UTILS_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

