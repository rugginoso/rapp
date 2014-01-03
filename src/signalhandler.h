/*
 * signalhandler.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

struct Logger;
struct ELoop;
struct SignalHandler;

typedef void (*SignalHandlerCallback)(struct SignalHandler *signal_handler, void *data);

struct SignalHandler *signal_handler_new(struct Logger *logger, struct ELoop *eloop);
void signal_handler_destroy(struct SignalHandler *signal_handler);

int signal_handler_add_signal_callback(struct SignalHandler *signal_handler, unsigned sig, SignalHandlerCallback callback, void *data);
int signal_handler_remove_signal_callback(struct SignalHandler *signal_handler, unsigned sig);

#endif /* SIGNALHANDLER_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

