/*inline*/
f (int x)
{
  int *p[13];
  int m[13*3];
  int i;

  for (i = 0; i < 13; i++)
    p[i] = m + x*i;

  p[1][0] = 0;
}

main ()
{
  f (3);
  exit (0);
}
