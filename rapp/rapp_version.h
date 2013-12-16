#ifndef RAPP_VERSION_H
#define RAPP_VERSION_H

// Increment this on introdution of ABI incompatibilities.
#define ABI_VERSION 1
int rapp_get_abi_version(void);
const char* rapp_get_version(void);
const char* rapp_get_version_full(void);
const char* rapp_get_version_sha1(void);
const char* rapp_get_version_tag(void);

#endif /* RAPP_VERSION_H */
