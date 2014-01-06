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


3. Control flow graph
---------------------

TBW


4. Resuming policy
------------------

TBW

