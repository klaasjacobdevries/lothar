The Origins of Lothar
=====================

As the difference between men and boys is after all only visible in some minor details I decided to
buy myself a Lego Mindstorms robotics kit. As a robot is essentially a pet, and a pet needs a name,
I christened it (or he? or she? I don't know. I don not have the methodology to figure it out)
Lothar!

As both pets and robots need to be taught some tricks I looked around for software to do exactly
that. As I found none that was *exactly* what I liked, I decided to write my own.

The Why? of Lothar
==================

As mentioned, the main and most important purpose for this library is - very egocentrically - to be
what I want it to be. There are quite a number of ways to program for your NXT 2.0 brick, and you
are encouraged to look around for them. Most however make a particular choice of programming
language, or even invent their own (typically something almost, but not exactly, like C). I found
none for my weapons of choice (which would be C++ or Python).

For that purpose I started out with the idea of a library that should be os-neutral and should be
language-neutral ('should' is of course an ominous word, later more on this). The core is written in
portable C, as C is probably closest to the common subset of languages you can get, with the idea
that it can easily be wrapped in other languages.

As to not be hindered by the limitations of the NXT brick itself your programs will run on your pc,
which use USB or Bluetooth to connect to the brick. There is no need to install custom firmware.

The How? of Lothar
=================

communications layer
--------------------

The main problem in the aim of being os-neutral is that USB- and Bluetooth communications are very
os-dependent in nature. For USB communications [LibUSB](http://www.libusb.org/) is used, which
should work among others for Linux, FreeBSD, Mac OSX and Windows. To my knowledge no such
cross-platform library exists for Bluetooth, for this we still rely on Linux-only
[Bluez](http://www.bluez.org/).

But all is not lost. If you are not afraid to get your hands a little bit dirty you can supply your
own communications mechanism (for Bluetooth or possibly some other mechanism). Supply your custom
vtable (who said C was not an object-oriented language?) to `lothar_connection_open_custom`, or
subclass `lothar::CustomConnection` if you're using C++.

commands layer
--------------

In `commands.h` you will find the calls for 'direct commands', the API the brick uses to directly
control the motors, sensors and various other capabilities of the brick. Typically you will want the
slightly higher-level motor- or sensor functions, but for other tasks like playing sound files this
is the only way.

motor/sensor layer
------------------

In `motor.h` and `sensor.h` you will find the higher-level methods to control the motors and sensors
on your brick, to you may of course still use the lower-level functions in the commands layer if you
need more fine-grained control.

odd ducks
---------

In `steering.h` you can find an experimental model to control the steering of a robot which steers by
varying the speed of its left and right wheels, while at the same time guesstimating its odometry.

In `scheduler.h` you can find an active scheduler to allow the easy (semi-)parallel execution of
tasks by the robot.

Both of these should arguably not be part of Lothar and should be implemented by another separate
library, but hey, they're there, you can use them.

wrapping
--------

If you prefer to reserve the use of C for other things, wrappers for C++ and Python can be found in
the wrappers/ subdirectory.

The triumph of Lothar
====================

Lothar has been demonstrated to work on Linux (USB and Bluetooth) and Mac OSX (USB only).

Examples of Lothar in action, using their Python wrappers and based on the ShooterBot and
ColorSorter standard models can be found in the examples/ subdirectory.

The trials of Lothar
====================

Now the 'should'. Theoretically Lothar 'should' work on every system supported by LibUSB. So what
does that mean? Applying the maxim that if it isn't tested, it doesn't work it means it doesn't
work.

It does mean however that there are no known reasons why it shouldn't work, so if you're enough of a
tinkerer you can get it to work. Good luck.

The sons of Lothar
==================

Lothar is of course not done. A none-exhaustive todo/wishlist:

* Testability on platforms other than Linux and Mac OSX.
* Bluetooth support for platforms other than Linux.
* [SWIG](http://www.swig.org/) appears to be a particularly cool way to provide wrappers for many,
  many languages. Pay special attention to the 'SWIG for the truly lazy' section in the tutorial.
