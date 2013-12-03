
    
         (
     )\ )   (
    (()/(   )\
     /(_)|(((_)(  `  )  `  )
    (_))  )\ _ )\ /(/(  /(/(
    | _ \ (_)_\(_|(_)_\((_)_\
    |   /  / _ \ | '_ \) '_ \)
    |_|_\ /_/ \_\| .__/| .__/
                 |_|   |_|
    

Overview
========
RApp is an application container, written in C.
It is focused on performances, targets GNU/Linux only and uses most of its own
features like epoll and signalfd.

Status
======
RApps is still not complete, currently it accepts connections, parses http
requests (by http_parser https://github.com/joyent/http-parser) and closes
connection.

Architecture
============
The base class is ELoop, the event loop which permits to watch a file
descriptor.

ELoop embeds another class, Collector, used to schedule the destruction of an
object at the end of the current loop.

TcpServer is, of course, a server TCP. It adds a watcher on the listening fd to
the event loop, which in turn calls a callback.
This callback accepts the connection and create a TcpConnection.

The TcpConnection wraps a file descriptor, and provides callbacks for the
readable and writable states of it.

HTTPServer implements the accept callback of the TcpServer and manages the life
of the HTTPConnection insstances.
When the accept callback is called, it creates a new HTTPConnection using the
TcpConnection provided by the TcpServer.

The HTTPConnection provides the callbacks for the TcpConnection, and contains
the http parser.
When data is received, it is parsed and the headers are copied in their own data
strctures.

For now, for every connection the headers are only printed on stdout and the
connection is closed.

License
=======
RApp is realeased under the GPLv2.
