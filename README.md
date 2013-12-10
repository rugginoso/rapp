         (
     )\ )   (
    (()/(   )\
     /(_)|(((_)(  `  )  `  )
    (_))  )\ _ )\ /(/(  /(/(
    | _ \ (_)_\(_|(_)_\((_)_\
    |   /  / _ \ | '_ \) '_ \)
    |_|_\ /_/ \_\| .__/| .__/
                 |_|   |_|


[![Build Status](https://secure.travis-ci.org/rugginoso/rapp.png?branch=develop)](https://travis-ci.org/rugginoso/rapp)

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

Coding Style
============

Indentation
-----------
2 spaces
new line at the end of file

Functions
---------
In function dichiaration parameters can be in the same line of the function:

```c
    int foo(struct Bar *object, int value, void *data);
```

But in function implementation return value is in its own line as well each parameter:

```c
   int
   foo(struct Bar *object,
       int         value,
       void       *data)
   {
     ...
   }
```

Note also the alignment of parameters types and names.

Braces
------
Braces must be at the right of the control structures

```c
    if (fd = open(path, O_RDONLY)) < 0) {
      ...
    }

    for (p = object->items; p != NULL; p = p->next) {
      ...
    }

    while (count) {
      ...
    }
```

But functions have braches on a new line

```c
    int
    foobar(void)
    {
      ...
    }
```

Variables
---------
Variables must have descriptive names, short names are allowed only for
automatic variables used in cicles.

Variables must be defined and initialized at the start of the function

```c
    int
    foobar(void)
    {
      int fd = -1;
      char *name = NULL;
      struct SomeObject *some_object = NULL;
      ...
    }
```

Assertions
----------
Use assert conditions on all function paramters on which makes sense. Assert expecially
pointers values.
Assertions on parameters are placed after variables dichiaration.

```c
   int
   foobar(struct SomeObject *object,
          char              *name)
   {
     int foo = 0;

     assert(object != NULL);
     assert(name != NULL);
     ...
   }

```

Objects
-------
RApp uses an OOP approach. Each object must have its own header and implemntation files, which must be named as the object they are defining.

Use opque structures for private data.

Methods must be named <object>_<action>, taking as the first parameter on <object> instance.

Each object must define an <object>_new mthod, returning an <object> instance, and a <object>_destroy method,
returning void and taking an <object> instance as the only parameter.


```c
   struct SomeObject;

   struct SomeObject *some_object_new(void);
   void some_object_destroy(struct SomeObject *some_object);

   void some_object_do_action(struct SomeObject *some_object, int foo);
```

Includes
--------
Includes should be grouped by scope, divided by one empty line.
System includes must come first, applications ones after.


```c
   #include <stdlib.h>
   #include <string.h>

   #include <sys/types.h>
   #include <sys/stat.h>

   #include "someotherobject.h"
   #include "someobject.h"
```

License
=======
RApp is realeased under the GPLv2.
