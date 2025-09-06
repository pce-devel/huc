#ifdef __HUCC__

#include <huc.h>

// Make sure that the #defpal crosses a bank boundary.

#asm
  .data
  .align 8192
  .ds 8192 - 3
  .code
#endasm

// N.B. "#defpal" declares mypal as an "extern int mypal[];"

#defpal( mypal, 0x111, 0x777, 0x555, 0x373 );

int main()
{
  unsigned int offset;

  offset = 2;

#if 0
  // Read as an array of the declared type.
  //
  // 2025-Sep-03 This __far array syntax isn't supported in HuCC anymore!

  if (mypal[0] != 0111)
    abort();

  if (mypal[1] != 0777)
    abort();

  if (mypal[2] != 0555)
    abort();

  if (mypal[offset] != 0555)
    abort();

  if (mypal[offset + 1] != 0733)
    abort();
#endif

  // Read directly from a __far pointer.

  if (farpeek( mypal ) != 0111)
    abort();
  if (farpeekw( mypal ) != 0111)
    abort();

  if (farpeek( mypal + 1 ) != 0377)
    abort();
  if (farpeekw( mypal + 1 ) != 0777)
    abort();

  if (farpeek( mypal + 2 ) != 0155)
    abort();
  if (farpeekw( mypal + 2 ) != 0555)
    abort();

  if (farpeek( mypal + offset ) != 0155)
    abort();
  if (farpeekw( mypal + offset ) != 0555)
    abort();

  if (farpeek( mypal + (offset + 1) ) != 0333)
    abort();
  if (farpeekw( mypal + (offset + 1) ) != 0733)
    abort();

  if (farpeek( mypal + offset + 1 ) != 0333)
    abort();
  if (farpeekw( mypal + offset + 1 ) != 0733)
    abort();

  // Read from a calculated pointer (with a byte-offset).

  set_far_base( BANK(mypal), mypal );

  if (far_peek() != 0111)
    abort();
  if (far_peekw() != 0111)
    abort();

  set_far_offset( 1 * 2, BANK(mypal), mypal );

  if (far_peek() != 0377)
    abort();
  if (far_peekw() != 0777)
    abort();

  set_far_offset( 2 * 2, BANK(mypal), mypal );

  if (far_peek() != 0155)
    abort();
  if (far_peekw() != 0555)
    abort();

  set_far_offset( offset * 2, BANK(mypal), mypal );

  if (far_peek() != 0155)
    abort();
  if (far_peekw() != 0555)
    abort();

  set_far_offset( (offset + 1) * 2, BANK(mypal), mypal );

  if (far_peek() != 0333)
    abort();
  if (far_peekw() != 0733)
    abort();

  return 0;
}

#else

/* HuC has problems with farpeekw() and does not support far_peekw() */
int main()
{
  return 0;
}

#endif
