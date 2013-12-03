#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

struct ELoop;
struct SignalHandler;

typedef void (*SignalHandlerCallback)(struct SignalHandler *signal_handler, void *data);

struct SignalHandler *signal_handler_new(struct ELoop *eloop);
void signal_handler_destroy(struct SignalHandler *signal_handler);

int signal_handler_add_signal_callback(struct SignalHandler *signal_handler, unsigned sig, SignalHandlerCallback callback, void *data);
int signal_handler_remove_signal_callback(struct SignalHandler *signal_handler, unsigned sig);

#endif /* SIGNALHANDLER_H */

