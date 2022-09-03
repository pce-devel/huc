*****************************************************************************
rom-core-okitest - OKI ADPCM tester HuCARD example
*****************************************************************************

This example is a test project used to investigate how the PCE's IFU and OKI
ADPCM chip behave when it comes to developing software for the PC Engine.

Two copies of an ADPCM sample are uploaded to the ADPCM RAM, and the rest of
the ADPCM RAM is filled with ADPCM silence ($08 = +1/-1).

One of the samples is read back from ADPCM RAM and compared with the original
HuCARD data to confirm that both the adpcm_read() and adpcm_check() functions
are working properly.

After it has been checked that the sample data is OK, then the tests execute.


The maximum safe transfer rate to/from ADPCM RAM was determined by modifying
the various alternative inner transfer loops of both functions, and checking
to see if the routines failed.

Notably, when you read or write a byte to the ADPCM RAM, then a BUSY flag is
set and the IFU hardware waits for the next access slot to write or read the
(next) byte from ADPCM RAM.

To determine when you can read/write the next byte, you can either check the
status of the flags and wait, or you can just wait for the maximum time that
the IFU needs to read/write the previous byte.

The adpcm_read() and adpcm_check() functions have a build-time option which
select which type of method to use, where the version that checks the flags
has been tuned so that the flag-test should never loop in normal operation.


There are two ADPCM PLAY FLAGS tests, and one ADPCM WRITE RATE test.

The ADPCM WRITE RATE test has been made mostly-obsolete by the testing, but
it still shows that the IFU's memory access slots for the read and write do
not change if a sample is playing, and that the playback rate does not make
a difference, which suggests that the IFU is using access-slots for timing
rather than responding immediately to CPU requests.


The 1st ADPCM PLAY FLAGS test plays back the sample four times, testing how
the ADPCM control and flag bits behave, which is then displayed.

Notably, the AD_BUSY flag (bit 3 of $180C) is delayed in both setting and
clearing compared to the AD_PLAY command bit (bit 5 of $180D), although it
seems unlikely that any game software relies on this.


The 2nd ADPCM PLAY FLAGS test plays back the sample once, and then manually
reads and writes bytes in order to check how the INT_HALF and INT_END flags
behave.

Notably, INT_END does NOT seem to be set when the LENGTH decrements to zero
on an manual read, but it is set (and INT_HALF cleared) on the decrement to
-1.

This contrasts to normal playback, where when INT_END is set on completion,
INT_HALF is still set.

Performing a manual read after a sample finishes will clear INT_HALF.

This suggests that the AD_AUTO control bit (bit 6 of $180D) may be stopping
the sample when the LENGTH == 0.

One curious thing to note is that setting a LENGTH of $7FFE and then writing
two bytes (so length should be $8000), does NOT clear the INT_HALF bit.

*****************************************************************************
