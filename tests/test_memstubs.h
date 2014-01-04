/*
 * test_memstubs.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef TEST_MEMSTUBS_H
#define TEST_MEMSTUBS_H


/* 
 * reset counter to ttl. Decrease by 1 at each memory_create.
 * when ttl <= 0, the next `fail_count` memory_create will return NULL.
 * for `fail_count`, -1 means forever.
 */
void memstub_failure_enable(int ttl, int fail_count);
void memstub_failure_disable(void);


#endif /* TEST_MEMSTUBS_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

