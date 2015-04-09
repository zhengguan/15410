#include <simics.h>
#include <syscall.h>
volatile int i;

void fuck_up() {
    i++;
    return;
}

int main() {
    *(int*)fuck_up = 0x0F0BF0B;
    fuck_up();
    return -1;
}
