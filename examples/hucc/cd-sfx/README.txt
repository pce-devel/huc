This example demonstrates how to play a simple sound effect file off of a CD.

Audacity (separate utility) was used to convert a wav file to 16k mono with a sample 
count that was a multiple of 4096.

Wav2vox (included with huc) was used to convert a 16k mono wav file to a vox file.

Pay special attention to the makefile as I had to specify an overlay to include the 
vox file within the iso.

The ad_trans call was tricky for me as I did not know the correct parameters. After 
some experimenting, I landed on the following. I knew this was the first included 
file but was not certain of the overlay index (ended up being 2). The 3 is the 
number of 2048 byte multiples that the vox file takes up.

ad_trans(2, 0x0000, 3, 0x0000);

The ad_play call was easier. I had to specify the number of bytes for the vox file 
and the frequency of the file, which was 16k.

ad_play(0x0000, 6144, ADPLAY_FREQ_16KHZ, ADPLAY_AUTOSTOP);

The sound effect is provided by kronbits. The original file can be found at the 
following location:

https://kronbits.itch.io/freesfx