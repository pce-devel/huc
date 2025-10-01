/*__volatile*/const int a = 0;

#ifdef __HUCC__
extern unsigned char __fastcall __builtin_ffs( unsigned int value<__temp> );
#endif

extern void abort (void);
extern void exit (int);

int
main (void)
{
  if (__builtin_ffs (a) != 0)
    abort ();
  exit (0);
}
