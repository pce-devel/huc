/***********************************************/
/* CDERR.C - Program to demonstrate            */
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
/* initialize: */

   set_xres(328);

   set_color(0,0);
   set_color(1,511);

   put_string("Demo Test Overlay", 8, 2);
   put_string("Program #4", 8, 3);
   put_string("Super System Card 3 required!", 2, 5);

   while (1) {
   }
}

