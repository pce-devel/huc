/***********************************************/
/* PROG3.C - Program to demonstrate            */
/*           Persistence of Globals and        */
/*           Loading program DATA from CDROM   */
/*                                             */
/* Written on August 25, 2001 by Dave Shadoff  */
/*                                             */
/***********************************************/

#include "huc.h"


/* Data exchange between program and  */
/* subroutines which execute from RAM */


/* Program variables */

const char dave_init[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

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

   put_string("Demo Test Overlay", 8, 2);
   put_string("Program #3", 8, 3);
   put_string("Press <START> for next program", 4, 4);
   put_string("Press <SEL> to load DATA from CD", 4, 5);

   while (1) {

      for (i = 0; i < 10; i++)
      {
         put_hex(dave_init[i], 2, 3*i, 15);
      }

      if (vram_loaded == 1)
      {
         put_string("You pressed <SEL> in program 2     ", 0, 17);
      }
      else
      {
         put_string("You didn't press <SEL> in program 2", 0, 17);
      }


      key = 0;

      while ((key != JOY_STRT) && (key != JOY_SLCT))
         key = joytrg(0);

      vsync();

      if (key == JOY_STRT)
      {
         cd_execoverlay(OVL_PROG1);
      }
      else
      {
         cd_loaddata(OVL_CHARDATA, 0, dave_init, 10);
      }
   }
}

