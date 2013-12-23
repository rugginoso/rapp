/*
 * rapp_version.h - is part of he public API of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_VERSION_H
#define RAPP_VERSION_H

// Increment this on introdution of ABI incompatibilities.
#define ABI_VERSION 1
int rapp_get_abi_version(void);
const char* rapp_get_version(void);
const char* rapp_get_version_full(void);
const char* rapp_get_version_sha1(void);
const char* rapp_get_version_tag(void);
const char* rapp_get_banner(void);

#endif /* RAPP_VERSION_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

