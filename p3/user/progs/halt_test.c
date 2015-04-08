#include <syscall.h>
#include <simics.h>

int main ()
{
    lprintf("pre-halt");
    halt();
    lprintf("post-halt");

    return 0;
}