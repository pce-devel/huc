*****************************************************************************
Turbo Everdrive V2 USBCD Overlay Tester
*****************************************************************************

Note: To build this HuCard ROM, you must find your own source for a HuCard
      image of the Japanese "Super System Card 3.0", and then copy it into
      this directory with the filename "SYSCARD3.JPN.PCE".

      You can confirm the validity of your file by checking its MD5 value.

      SYSCARD3.JPN.PCE - MD5: 38179df8f4ac870017db21ebcbf53114
      SYSCARD3.USA.PCE - MD5: 0754f903b52e3b3342202bdafb13efa5

      Because you must provide your own file, this project is not built by
      default, and you must navigate to this directory and "make".

*****************************************************************************

This project is a tool for those PCE developers lucky-enough to have a Turbo
Everdrive V2 with a USB port, and it is not much use for anyone else.

The project creates a customized version of the Super System Card that can be
used to test homebrew SCD overlay programs created by PCEAS.

To do that ...

  1) "make" this project to create the HuCard image "ted2usbcd.pce".

  2) "make" your own game's SuperCD overlay using PCEAS as normal.

  3) Concatenate both files into a single test HuCard image.

     In a Windows Command Prompt, this can be done with ...
     "copy /b ted2romcd.pce + mygame.ovl test.pce"
     
  3) Upload ted2usb.pce to your TED2 with Krikzz's "turbo-usb.exe".

     The System Card will startup, then automatically boot the CD that is
     in the drive, then it waits for you to upload a CD overlay to run.

  4) Upload your game's SuperCD overlay with Krikzz's "turbo-usb.exe".

     This avoids the need to burn a CD-R disc every time that you want to
     test some code on a real PC Engine CD-ROM.

*****************************************************************************
