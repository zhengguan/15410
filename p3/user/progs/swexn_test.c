#include <syscall.h>
#include <simics.h>
#include <ureg.h>
#include <stdlib.h>

void handler(void *arg, ureg_t *ureg)
{
    lprintf("IN HANDLER, %u", (unsigned)arg);
    MAGIC_BREAK;
}


int main() {
    int err;

    char *stack = malloc(8192);
    if ((err = swexn(stack, handler, (void*)12, 0)) < 0)
        lprintf("swexn failed: %d", err);
    *(char*)0 = 'a';
    return 0;
}