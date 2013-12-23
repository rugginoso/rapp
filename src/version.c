#include <string.h>
#include <config.h>
#include <rapp/rapp_version.h>

const char*
rapp_get_version_full(void)
{
    return FULL_VERSION;
}

const char*
rapp_get_version(void)
{
    return VERSION;
}

const char*
rapp_get_version_sha1(void)
{
    return GIT_SHA1;
}

const char*
rapp_get_version_tag(void)
{
    if (strcmp(GIT_TAG, "-128-NOTFOUND") == 0)
        return "";
    return GIT_TAG;
}

const char*
rapp_get_banner(void)
{
  return BANNER;
}
