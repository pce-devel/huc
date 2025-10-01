#include <string.h>
extern void abort(void);

int foo(const char *a)
{
    return strcmp(a, "main");
}

int main(void)
{
    if(foo(__FUNCTION__))
        abort();
    return 0;
}
