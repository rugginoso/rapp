#!/usr/bin/python
#
# put in the public domain
# usage:
# $ cat files_to_decorate | /path/to/cdeco.py

import os
import os.path
import sys


def decorate(srcpath):
    srcname = os.path.basename(srcpath)
    header = [
        "/*\n",
        " * %s - is part of RApp.\n" % srcname,
        " * RApp is a modular web application container made for linux and for speed.\n",
        " * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.\n",
        " *     see LICENSE for all the details.\n",
        " */\n"
    ]
    trailer = [
        "/*\n",
        " * vim: expandtab shiftwidth=2 tabstop=2:\n",
        " */\n"
    ]
    with open(srcpath, "rt") as src:
        content = src.readlines()
    blankline = ["\n"]
    separator = [] if content[0] == "\n" else blankline
    with open(srcpath, "wt") as dst:
        dst.writelines(header
                     + separator
                     + content
                     + trailer
                     + blankline)


def _main():
    for src in sys.stdin:
        decorate(src.strip())


if __name__ == "__main__":
    _main()
