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
RApp is still not complete, currently it accepts connections, parses http
requests (using joyent's [http_parser](https://github.com/joyent/http-parser))
and closes connection.


Architecture
============
![Class Diagram](http://yuml.me/f8c29e8b)


Dependencies
============
RApp requires a modern (>= 2013, kernel >= 3.8) Linux system to build and run.
RApp depends on an handful of external packages:

* [libYAML](http://pyyaml.org/wiki/LibYAML) to handle the configuration.
* [check](http://check.sf.net) for the testsuite.


Compile
=======
RApp uses [cmake](http://www.cmake.org/). Once you have cloned this repository
and installed cmake run:
```bash
    $ git submodule init
    $ git submodule update
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
```
Then you can start RApp using:
```bash
    $ cd build
    $ ./src/rapp
```

License
=======
RApp is realeased under the GPLv2.
