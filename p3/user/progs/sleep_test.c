#include <simics.h>
#include <syscall.h>

#define SLEEP_TICKS 10
#define DELAY (128*1024)

int patrick = 0;

void foo() {
  ++patrick;
}

/* this function eats up a bunch of cpu cycles */
void slow() {
  int i;
  for (i = 0; i < DELAY; i++) foo();
}

int main() {
    if (fork() == 0) {
        lprintf("Sleeping at %d ticks, for %d ticks.", get_ticks(), SLEEP_TICKS);
        sleep(SLEEP_TICKS);
        int woken = get_ticks();
        while(1) {
            lprintf("Sleeping Beauty woken at %d ticks.  Current ticks %d", woken, get_ticks());
            slow();
        }
    } else {
        while(1) {
            lprintf("Snow White");
            slow();
        }
    }
    return 0;
}
