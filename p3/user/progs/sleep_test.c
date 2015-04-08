#include <simics.h>
#include <syscall.h>

#define SLEEPING_BEAUTY 50
#define SNOW_WHITE 10
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
        lprintf("Sleeping Beauty: sleeping for %d ticks", SLEEPING_BEAUTY);
        int ret = sleep(SLEEPING_BEAUTY);
        if (ret < 0) {
            lprintf("Sleeping Beauty: failed with error %d", ret);
        } else {
            lprintf("Sleeping Beauty: woken at %d ticks", get_ticks());
        }
        while(1) {
            slow();
        }
    } else {
        if (fork() == 0) {
            lprintf("Snow White: sleeping for %d ticks", SNOW_WHITE);
            int ret = sleep(SNOW_WHITE);
            if (ret < 0) {
                lprintf("Snow White: failed with error %d", ret);
            } else {
                lprintf("Snow White: woken at %d ticks", get_ticks());
            }
            while(1) {
                slow();
            }
        } else {
            while(1) {
                slow();
            }
        }
    }
    return 0;
}
