/* Test for C99 __func__.  */
/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do run } */
/* { dg-options "-std=iso9899:1999 -pedantic-errors" } */

#include <string.h>

extern void abort (void);
extern void exit (int);

int
main (void)
{
  if (strcmp (__func__, "main") || sizeof (__func__) != 5)
    abort ();
  else
    exit (0);
}
