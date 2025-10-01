/* REPRODUCED:RUN:SIGNAL MACHINE:i386 OPTIONS:-O */
#include <string.h>

main()
{
if(strcmp("X","")<0)abort();
exit(0);
}
