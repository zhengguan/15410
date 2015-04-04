#include <simics.h>
#include <syscall.h>

#define BUF_SIZE 80

#define DELAY (4096*1024)

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
        set_term_color(BGND_CYAN | FGND_YLLW);
        char buf[BUF_SIZE];
        while(1) {
            int len = readline(BUF_SIZE, buf);
            lprintf("Return: %d", len);
            lprintf("Return: %s", buf);
            print(len, buf);
        }
    } else {
        while(1) {
            slow();
        }
    }
    while(1);

    return 0;

}
