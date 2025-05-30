# CREATING LARGE MAPS FOR A GAME LEVEL

## Copyright Notice

Here are some of graphics files for the Seran Village map and its surrounding
level map from Nihon Falcom's "Legend of Xanadu 2" game, providing an example
of how a master video game creator would create large level data back then.

Their artwork is used under the "fair use exemption" to copyright in order to
teach modern developers how create modern games using similar techniques.

The artwork itself may not be used in commercial games without Nihon Falcom's
express written permission.


## The Techniqes Used

Falcom split their "Legend of Xanadu 2" levels into 1024x1024 pixel sections,
blanking the screen and loading new graphics and palette data when the player
moved from one section to the next.

Each section map was created using 16x16 blocks (meta-tiles) that are created
from multiple sets of 256 8x8 characters (tiles) each taking 8KBytes of VRAM.

There were 3 sets of characters loaded for each map section, 1 set for shared
characters used by many different map sections, and 2 for characters that are
unique to the current map section, often with 1 set for the static background
graphics and 1 set for animated background characters.


## The Individual Files

* blank.png
  - A simple 16x16 blank image that can be used to import a blank tile or sprite.


* seran-hit.png
  - This is the Seran Village collision map.

* seran-map.png
  - This is the Seran Village background map.

* seran-msk.png
  - This is the Seran Village mask-sprite map.

* seran-shore.gif
  - This shows how the Seran Village shoreline animation is made up from 4 frames of animated characters.

* seran-shore.png
  - These are the 4 frames of the Seran Village shoreline animation as an image ready to be converted.

* seran-water.gif
  - This shows how the Seran Village water animation is made up from 8 frames of animated characters.

* seran-water.png
  - These are the 4 frames the Seran Village water animation as an image ready to be converted.


* rpg-west-map.png
  - This is the Western 3 sections of a 5 section background map.
  - The "shore" section uses palettes 0,1 and 10,11,12,13.
  - The "seran" section uses palettes 0,1 and 2,3,,5,6,7,8.
  - The "cliff" section uses palettes 0,1 and 14.

* rpg-west-hit.png
  - This is the Western 3 sections of a 5 section collision map.

* rpg-west-msk.png
  - This is the Western 3 sections of a 5 section mask-sprite map.

* rpg-west-basic.png
  - This is the Western 3 sections of a 5 section background map stripped down to a 256 characters that are shared.
  - It is an example of how you might progressively strip away features from a map in order to make paritioned character sets.

* rpg-west-extra.png
  - This is the Western 3 sections of a 5 section background map stripped down to a 512 characters that are shared.
  - It is an example of how you might progressively strip away features from a map in order to make paritioned character sets.


* rpg-east-map.png
  - This is the Eastern 3 sections of a 5 section level map.
  - The "cliff" section uses palettes 0,1 and 14.
  - The "river" section uses palettes 0,1 and 7,8,9,10,11,12,13.

* rpg-east-hit.png
  - This is the Eastern 3 sections of a 5 section collision map.

* rpg-east-msk.png
  - This is the Eastern 3 sections of a 5 section mask-sprite map.


* river-shore.gif
  - This shows how the River shoreline animation is made up from 3 frames of animated characters.
  - The shore animation is a cycle of frames 0,1,0,2.
  - It also shows how the upper and lower parts of the waterfalls are made up from 8 frames of animated characters,
  - and the froth is made up from  frames of animated characters.

* river-shore.png
  - These are the 3 frames of the River shoreline animation as an image ready to be converted.

* river-froth.png
  - These are the 8,8,4 frames of the River froth animation as an image ready to be converted.

* river-falls.png
  - This is 32x32 pixel color-cycled texture used for the center of the smaller waterfalls.

* river-water.gif
  - This shows how the River water animation is made up from 8 frames of animated characters.

* river-water.png
  - These are the 8 frames the River water animation as an image ready to be converted.
