/***********************************************/
/* PROG1.C - Program to demonstrate            */
/*           CD Overlays                       */
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

   put_string("Demo Test Overlay", 8, 2);
   put_string("Program #1", 8, 3);

   put_string("Press <START> for next program", 4, 4);
   put_string("                                      ", 4, 5);

   while (1) {

      while((joytrg(0) != JOY_STRT))
         vsync(0);

      vsync(0);

      cd_execoverlay(OVL_PROG2);
   }
}

