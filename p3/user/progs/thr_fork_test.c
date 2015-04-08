#include <simics.h>
#include <syscall.h>
#include <thread.h>

void *thr1(void *arg)
{
    lprintf("hello from %d", thr_getid());

    return (void*)thr_getid();
}


int main() {

    thr_init(PAGE_SIZE);

    lprintf("hello from %d", thr_getid());
    thr_create(thr1, 0);
    lprintf("hello from %d", thr_getid());
    return 0;
}
