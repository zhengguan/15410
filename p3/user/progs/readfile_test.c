#include <simics.h>
#include <syscall.h>
#include <string.h>

int main() {

    char buf[1000];

    int offset = 0;

    while ( readfile(".", buf, 1000, offset) > 0) {
        lprintf("\"%s\"", buf);
        offset += strlen(buf) + 1;
    }

    while (1);
}
