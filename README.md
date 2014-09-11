README
------

nekostring is an emulation of a 70s-era string ensemble.  It has four
oscillators per voice ("Bass", "Cello", "Viola" and "Violin"), and a chorus
ensemble effect to thicken up the sound.  It's not based on a Solina even a little bit.

This version is about as early as it gets and still can be called working.
The code is heavily based on Xsynth-DSSI, and you can see a lot of redundant
code for that in here.  The GUI doesn't work at all.  The code does not build
cleanly.

In order to adjust the sounds, you will need to use `dssi_osc_send` or similar.
The only values that can be adjusted at the moment are:

    3: Cello level (0-1)
    4: Viola level (0-1)
    5: Violin level (0-1)

    10: Attack time (0-1, 0 being slowest)
    11: Decay time (0-1, 0 being slowest)

    14: Volume (0-1)
    15: Tuning (420-460 Hz)

To use nekostring, you need some sort of DSSI host. I use jack-dssi-host. What you want is something like `jack-dssi-host nekostring.so` - the GUI should appear and you should be able to start sending stuff to its midi port.

The first release will at least have the Ensemble effect controllable over DSSI.

INSTALLATION
------------

You need to install the following dependencies. They may have different names on different distros.

 * build-essential
 * ladspa-sdk
 * dssi-dev
 * liblo-dev
 * libgtk2.0-dev

There might be a few more on your system, depending on what you've already installed.

Next, you want to run `autoreconf` until the only thing it complains about is the `_SOURCES` line in `gui/Makefile.am`. You may need to run `libtoolize` first. Automake is a confusing and awful thing.

Finally, you will be able to run the following commands:

    $ make
    $ make check # tests
    $ sudo make install

If you get errors, stop and diagnose them, rather than trying to bull ahead. At the end of it, you should have a nekostring.so library that you can use in a DAW or via JACK.

Copyright information 
---------------------

This plugin is derived from Sean Bolton's Xsynth-DSSI, which in turn contains
code from S. J. Brookes' Xsynth.  Parts of the oscillator code come from Fons
Adriaensen's VCO-Plugins, by way of Xsynth-DSSI.  Parts of the DSSI handling
code may have come from Fluidsynth, again via Xsynth-DSSI.  The chorus/ensemble
plugin began life as part of Giant Flange by Steve W. Harris, but has changed
almost out of all recognition.  These programs are licenced under the GPL,
except where stated.  The various nekosynth plugins are also licenced under the
GPL.  Please see the attached file COPYING for further details, or contact the
Free Software Foundation for your very own copy.

This software is distributed without any warranty.  It may be suitable for a particular purpose; it may be entirely unfit for any purpose.  You use or misuse this software entirely at your own risk.  If you break it, you own both pieces.  This software may contain small parts which could present a choking hazard.  Do not use this software in wet conditions.  May cause drowsiness.  Do not use this software while operating heavy machinery, except under expert supervision.

Have fun!
