const char a[10] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69};

main()
{
  int  u,v,w;
  char x,y,z;
  u = 1;
  v = 5;
  w = a[u+v];
  if (a[u+1] != 62)
    exit(1);
  if (a[v] != 65)
    exit(2);
  if (w != 66)
    exit(3);
  x = 2;
  y = 3;
  z = a[x+y];
  if (a[x+1] != 63)
    exit(4);
  if (a[y] != 63)
    exit(5);
  if (z != 65)
    exit(6);
  return 0;
}
