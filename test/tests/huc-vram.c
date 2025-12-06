#include <huc.h>

const char foo[] = {1, 2, 3, 4, 5, 6};
int main()
{
  load_vram(0, foo, 3);
#ifdef __HUCC__
  put_vram(2, 6);
  if (get_vram(0) != 0x0201 || get_vram(1) != 0x0403)
    abort();
  if (get_vram(2) != 6)
    abort();
#else
  vram[3] = 6;
  if (vram[0] != 0x0201 || vram[1] != 0x0403)
    abort();
  if (vram[3] != 6)
    abort();
#endif
  return 0;
}
