#include <huc.h>

#ifdef INIT_GLOBAL2
int a = 2;
#else
int a;
#endif

extern char _huc_rodata;
extern char _huc_rodata_end;
extern char _huc_data;
int main()
{
  load_default_font();
  put_string("Lo", 0, 2);
  put_hex(&_huc_rodata, 4, 0, 3);
  put_hex(&_huc_rodata_end, 4, 5, 3);
  put_hex(&_huc_data, 4, 10, 3);
  put_number(a, 5, 15, 3);
  for(;;);
}
