# example programs

This directory contains a number of example programs and utilities for developing on the PC Engine in assembly-language.


## Utilities

* ipl-scd
  - A customized CD-ROM IPL that either displays an error message or runs an error overlay, when not booted on a Super CD-ROM.

* rom-ted2romcd
  - A modified System Card 3.0 HuCARD that runs a CD-ROM overlay stored inside the HuCARD image, rather than loading it from CD-ROM.


## Using the System Card BIOS

* scd-bios-hello
  - A simple example of creating an ISO for a Super CD-ROM, and using the System Card BIOS.


## Getting started with the "CORE(not TM)" PC Engine library

The "CORE(not TM)" PC Engine library is a small and configurable set of library routines, using less than 400 bytes at a minimum, that sets up a consistent PC Engine environment for both HuCARD and CD-ROM, and which provides interrupt handling, joypad reading, and file loading (on CD-ROM).


* rom-core-hello
  - A simple example of creating a HuCARD ROM.

* scd-core-1stage
  - A simple example of creating an ISO for a Super CD-ROM, and loading the "CORE(not TM)" kernel code from the overlay program.

* scd-core-2stage
  - A simple example of creating an ISO for a Super CD-ROM, and loading the "CORE(not TM)" kernel code in a startup overlay.
