#include <string.h>
char *f() {
#ifdef __HUCC__
  // HuC can't handle this ...
  strcmp("bad","wolf");
#endif
  return "f";
}
char *g() {
#ifdef __HUCC__
  // HuC can't handle this ...
  strcmp("bad","wolf");
#endif
  return "g";
}
char *h() {
  strcmp("bad","wolf");
  return "h";
}

#asm
_hfc		.proc
		call	_h
	.ifdef	HUCC
		tax
	.endif
		leave
		.endp
#endasm

char * __fastcall hfc(void);

main()
{
  if (!strcmp(f(), g()))
    abort();
  if (strcmp(f(), f()))
    abort();
  if (strcmp(f(), "f"))
    abort();
  if (!strcmp(g(), "f"))
    abort();
  if (strcmp("f", f()))
    abort();
  if (!strcmp("g", f()))
    abort();
#ifdef __HUCC__
  // HuC doesn't handle this correctly ...
  if (strcmp("h", hfc()))
    abort();
#endif
  return 0;
}
