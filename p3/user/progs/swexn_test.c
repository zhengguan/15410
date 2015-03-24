#include <syscall.h>
#include <simics.h>
#include <ureg.h>
#include <stdlib.h>

char *character = (char*)0x01100000u;

void handler(void *arg, ureg_t *ureg)
{
    lprintf("IN HANDLER, %u", (unsigned)arg);
    new_pages(character, PAGE_SIZE);
}


int main() {
    int err;

    char *stack = malloc(8192);
    if ((err = swexn(stack, handler, (void*)12, 0)) < 0)
        lprintf("swexn failed: %d", err);
    *character = 'a';
    lprintf("%c", *character);
    MAGIC_BREAK;
    return 0;
}