/***********************************************/
/* PROG2.C - Program to demonstrate            */
/*           Loading VRAM from CDROM           */
/*                                             */
/* Written on August 25, 2001 by Dave Shadoff  */
/*                                             */
/***********************************************/

#include "huc.h"


/* Data exchange between program and  */
/* subroutines which execute from RAM */


/* Program variables */


/****************/
/* MAIN section */
/****************/

main()
{
   int ptr, line;
   int i, key;

/* initialize: */

   set_xres(328);

   set_color(0,0);
   set_color(1,511);

   vram_loaded = 0;

   put_string("Demo Test Overlay", 8, 2);
   put_string("Program #2", 8, 3);
   put_string("Press <START> for next program", 4, 4);
   put_string("Press <SEL> to load VRAM from CD", 4, 5);

   while (1) {

      key = 0;

      while ((key != JOY_STRT) && (key != JOY_SLCT))
         key = joytrg(0);

      vsync();

      if (key == JOY_STRT)
      {
         cd_execoverlay(OVL_PROG3);
      }
      else if (key == JOY_SLCT)
      {
         vram_loaded = 1;
         cd_loadvram(OVL_VIDEODATA, 0, 0x0200, 0x0200);
      }
   }
}

