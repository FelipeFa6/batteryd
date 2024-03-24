batteryd - battery daemon for openbsd
============================

battery state watcher daemon for openbsd. runs a
script or an application when the battery state
switches to `high`, `low`, or `critical`.

usage
------------
batteryd [-d]

by default, `batteryd` runs as a daemon. if passed
the `-d` flag, `batteryd` enters debug mode,
staying in the foreground, and logging output is
printed to `stderr`.

batteryd polls `apm(4)` every second to discover
changes in the battery charge level state. three
states are handled: `critical`, `low`, and `high`.
when a change is discovered, an executable bearing
the name of the state located in the configuration
directory is executed.

installation
------------
compile and copy binary to $PATH:
$ make clean install

uninstall
------------

remove generated files on compile time:
    $ make clean 

remove from $PATH and generated files
    $ make clean uninstall

running batteryd
-----------

add the following line to your .xinitrc to start
batteryd:

    batteryd &

make sure to append it before your display manager
is executed

configuration
------------
The configuration of batteryd is done by creating
a custom config.h and (re)compiling the source
code.
