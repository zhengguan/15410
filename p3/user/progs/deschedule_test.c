#include <simics.h>
#include <syscall.h>

int main() {
    int a = 0;
    lprintf("trying to deschedule");
    lprintf("deschedule returned %d", deschedule(&a));
    return 0;
}
