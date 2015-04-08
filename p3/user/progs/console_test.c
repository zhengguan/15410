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
        char buf[BUF_SIZE];
        lprintf("Child");
        MAGIC_BREAK;
        while(1) {
            int len = readline(BUF_SIZE - 1, buf);
            buf[len] = '\0';
            lprintf("Return: %d", len);
            lprintf("Return: |%s|", buf);
            print(len, buf);
            MAGIC_BREAK;
        }
    } else {
        lprintf("Parent");
        MAGIC_BREAK;
        while(1) {
            slow();
        }
    }

    return 0;
}
