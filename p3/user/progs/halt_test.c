#include <syscall.h>
#include <simics.h>

int main ()
{
    lprintf("pre-halt");
    halt();
    lprintf("post-halt");

    while(1);
}