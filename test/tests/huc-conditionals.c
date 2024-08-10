#include <huc.h>

unsigned int a = 3;
unsigned int b = 4;
unsigned int c = 5;
unsigned int d = 0;
unsigned int count = 0;

int
try_zero( void )
{
  if ((0 == 0) != 1)
    abort();
  if ((1 != 0) != 1)
    abort();
  if (!(0 == 0) == 1)
    abort();
  if (!(1 != 0) == 1)
    abort();
  return 0;
}

int
try_and( void )
{
  if (a == 3 && b == 4 && c )
    return 0;
  abort();
}

int
try_or( void )
{
  if (a != 3 || b != 4 || !c )
    abort();
  return 0;
}

int
try_both( void )
{
  if ((c && !c) || a != 3 && b == 4)
    abort();
  if ((c || !c) && (a != 3 || b != 4))
    abort();
  if ((!c || d) && (a != 3 || b == 4))
    abort();
  return 0;
}

int
try_assign( void )
{
  int e;
  if ((e = (d = a == 3) && b == 4) != 1)
    abort();
  return 0;
}

int
increment( void )
{
  return count++;
}

int
try_early( void )
{
  if (increment() && increment())
    abort();
  if (count != 1)
    abort();
  if (increment() || abort())
    count++;
  if (count != 3)
    abort();
  return 0;
}

unsigned int
main( void )
{
  try_zero();
  try_and();
  try_or();
  try_both();
  try_assign();
  try_early();

  return 0;
}
