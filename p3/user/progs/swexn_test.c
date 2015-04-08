#include <syscall.h>
#include <simics.h>
#include <ureg.h>
#include <stdlib.h>

char *character = (char*)0x01100000u;
char *stack;

void handler(void *arg, ureg_t *ureg)
{
    lprintf("IN HANDLER, %u", (unsigned)arg);
    new_pages(character, PAGE_SIZE);
    swexn(stack, handler, (void*)11, ureg);
}


int main() {
    int err;

    stack = malloc(8192);
    if ((err = swexn(stack, handler, (void*)12, 0)) < 0)
        lprintf("swexn failed: %d", err);
    *character = 'a';
    lprintf("%c", *character);
    return 0;
}