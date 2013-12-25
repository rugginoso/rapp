/*
 * path.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef PATH_H
#define PATH_H

int get_path(char *path, struct HTTPRequest *http_request, const char *document_root);

#endif /* PATH_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

