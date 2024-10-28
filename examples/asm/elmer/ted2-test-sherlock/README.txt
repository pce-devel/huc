*****************************************************************************
Turbo Everdrive V2 "Sherlock Holmes Consulting Detective" (USA) Logfile Hack
*****************************************************************************

Note: To build this project, you must first put a copy of the CD's boot code
      into this directory with the filename "sherlock-boot.bin".

      If you have the ability to save binary files from within your emulator
      software, you can run the game's CD with a breakpoint at address $2A00
      and then save the memory from $2A00..$81FF as "sherlock-boot.bin".

      If you have the game's CD image dumped as a MODE1/2048 .iso file, then
      you can extract the file directly from the image itself using "dd" ...

        dd bs=2048 skip=2 count=11 if=<imagefile.iso> of=sherlock-boot.bin

      You can confirm the validity of your file by checking its MD5 value.

      sherlock-boot.bin - MD5: 60aebcac752ba7b39d133c5d45a44d85

      Because you must provide your own file, this project is not built by
      default, and you must navigate to this directory and "make".

*****************************************************************************
