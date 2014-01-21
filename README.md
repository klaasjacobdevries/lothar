Background
==========

Please read the Lothar document.

Building and installing
=======================

dependencies
------------

* [CMake](http://www.cmake.org/)
* [LibUSB 1.0](http://www.libusb.org/)
* (optional, Linux only) [Bluez](http://www.bluez.org/)
* (optional) [Python](http://www.python.org/)
* (optional) [Google Test and Google Mock](http://code.google.com/p/googlemock/)
* (optional) [Doxygen](http://www.stack.nl/~dimitri/doxygen/)

compiling
---------

Lothar uses CMake to generate project files for most popular IDES. To produce Makefiles for a
typical unix-like system do something like:

    $ cmake /path/to/source
    $ make
    $ make install

If you have Google Test and Google Mock installed (this should be automagically detected by CMake),
you can now also run the unit tests if you want:

    $ ./lothar_tests

Or generate the documentation with:

    $ make doc

And you can install the python module:

    $ python setup.py build
    $ python setup.py install

playing
=======

To start writing your applications, Lothars core program is written in C. Its headers should have
been installed $PREFIX/include/lothar (.h denote C headers, .hh denote C++ headers) and the library
in $PREFIX/lib, make sure you link against it (i.e. pass -llothar to your linker).

If you prefer something jazzier than C, wrappers are provided for C++ and Python. For C++ you can
find the headers in the same place (the .hh files are for C++), and you need to pass -llothar++ to
your linker.

For Python some examples - for the ShooterBot and the ColorSorter standard models - are included in
examples/ in the source tree.

