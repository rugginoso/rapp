Coroutines in RApp
==================

This document explains why RApp uses coroutine, what problem they solve
and their impact on the architecture.

If you want to refresh what coroutines are:

* [explanation and further links](http://en.wikipedia.org/wiki/Coroutine)

* [usage in C and simple implementation](http://www.embeddedrelated.com/showarticle/455.php)


1. Scenario and Context
-----------------------

When RApp succesfully extracted a full HTTP Request from a Connection,
it invokes the registered container for the route matched by the Request
itself.
When the container serves such a Request, it delivers the output in a HTTP
Response object, because the Connection itself is not exposed to the
Container.

The default behaviour of the Response object is to do heavy use of buffering
in order to improve performances and minimize the syscall round trip and load.
The Container may needs to control the buffering done by the Response, asking
to deliver into the Connection the data just produced as soon as possible,
of course modulo the Operating System settings outside of the control of RApp.

Bear in mind that RApp is a single-threaded, event-based server, and that
the containers are invoked by the event loop once their reference Connection
is ready.

Please note that this problem lies identical even if the Connection would be
exposed directly to the Container.
It is an execution-flow problem more than an abstraction-layer problem.

In order to rearrange the execution flow in order to support the response
flushing, RApp makes uses of coroutines.


2. Evaluation of the coroutine libraries
----------------------------------------

Requisites:

RApp will use N+1 coroutines, where N is the number of registered container
for any given run. Plus the one for the main thread, which also runs the
event loop.

Given the purpose of RApp, the library should offer reasonnable figures
for performance (the higher the better) and resource consumption
(the lower the better).

A Container may run arbitrarily complex code, so no safe assumption can be
made about their resource (read: stack) usage.
For the sake of safety, the coroutine library must then support
stack-swapping; coroutine must *not* share the stack.

The library must be available on linux/x86_64.
Support for other archs or OSes, except for the ubiquitous i386, is a
nice extra and should be consider as that for the evaluation.


[picoro](http://dotat.at/cgi/git/picoro.git)
small, nicely implemented, very liberal license. No explicit stack management.

[libtask](http://swtch.com/libtask/)

[pcl](http://xmailserver.org/pcl.html)

[lthread](https://github.com/halayli/lthread)

[coroutine](https://github.com/stevedekorte/coroutine/tree/master) from Steve Dekorte

[libconcurrency](http://code.google.com/p/libconcurrency/)

[libcoro](http://software.schmorp.de/pkg/libcoro.html)




3. Control flow graph
---------------------

TBW


4. Resuming policy
------------------

TBW

