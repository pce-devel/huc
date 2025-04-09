# example programs

This directory contains a number of example programs and utilities for developing on the PC Engine in assembly-language.


## Utilities

* ipl-scd
  - A customized CD-ROM IPL that either displays an error message or runs an error overlay, when not booted on a Super CD-ROM.

* ted2-bios-romcd
  - A modified System Card 3.0 HuCARD that runs a CD-ROM overlay stored inside the HuCARD image, rather than loading it from CD-ROM.

* ted2-bios-usbcd
  - A modified System Card 3.0 HuCARD that runs a CD-ROM overlay uploaded through USB, rather than loading it from CD-ROM.

* rom-bare-buftest
  - A HuCARD ROM to compare a PC Engine emulator's buffered VDC reading with real PC Engine console hardware.

* rom-bare-dmatest
  - A HuCARD ROM to compare a PC Engine emulator's VDC SATB-DMA and VRAM-DMA timing with real PC Engine console hardware.

* rom-bare-mwrtest
  - A HuCARD ROM to compare a PC Engine emulator's VDC VRAM read/write timings with real PC Engine console hardware.

* rom-bare-rcrtest
  - A HuCARD ROM to compare a PC Engine emulator's RCR interrupt handling with real PC Engine console hardware.

* rom-bare-tiatest
  - A HuCARD ROM to compare a PC Engine emulator's TIA-to-VDC cycle timing with real PC Engine console hardware.

* rom-core-okitest
  - A HuCARD ROM to compare a PC Engine emulator's ADPCM playback flags and ADPCM write speed with real PC Engine console hardware.


## Getting started with a HuCARD

* rom-bare-hello
  - A simple example of creating a HuCARD ROM.


## Using the System Card BIOS

* scd-bios-hello
  - A simple example of creating an ISO for a Super CD-ROM, and using the System Card BIOS.

* scd-bios-hello-error
  - An expansion of scd-bios-hello, using an error overlay to show a pretty screen when run with System Card 2.0 or earlier.


## Getting started with the "CORE(not TM)" PC Engine library

The "CORE(not TM)" PC Engine library is a small and configurable set of library routines, using less than 400 bytes at a minimum, that sets up a consistent PC Engine environment for both HuCARD and CD-ROM, and which provides interrupt handling, joypad reading, and file loading (on CD-ROM).


* rom-core-hello
  - A simple example of creating a HuCARD ROM.

* rom-core-hugerom
  - A simple example of creating a HuCARD ROM for the Street Fighter II mapper.

* cd-core-1stage
  - A simple example of creating an ISO for a CD-ROM, and loading the "CORE(not TM)" kernel code from the overlay program.

* cd-core-2stage
  - A simple example of creating an ISO for a CD-ROM, and loading the "CORE(not TM)" kernel code in a startup overlay.

* cd-core-scsitest
  - A CD-ROM and a HuCARD for testing some low-level details of the PC Engine's SCSI CD-ROM behavior.

* scd-core-1stage
  - A simple example of creating an ISO for a Super CD-ROM, and loading the "CORE(not TM)" kernel code from the overlay program.

* scd-core-2stage
  - A simple example of creating an ISO for a Super CD-ROM, and loading the "CORE(not TM)" kernel code in a startup overlay.

* scd-core-1stage-error
  - An expansion of scd-core-1stage, using an error overlay to show a pretty screen when run with System Card 2.0 or earlier.

* scd-core-2stage-error
  - An expansion of scd-core-2stage, using an error overlay to show a pretty screen when run with System Card 2.0 or earlier.

* scd-core-fastcd
  - An example of CD-ROM initialization, reading files and streaming ADPCM using the "fast" CD routines.

* ted2-core-hwdetect
  - An example of detecting PC Engine hardware and testing the Joypads and Mice attached to the console.

* ted2-core-sdcard
  - An example of reading and writing a file from the SDcard on a Turbo EverDrive v2.

* ted2-core-gulliver
  - An example of playing a HuVIDEO movie from Hudson's GulliverBoy CD-ROM game.
