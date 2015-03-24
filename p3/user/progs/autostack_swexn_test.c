#include <syscall.h>
#include <string.h>
#include <simics.h>

#define BIG_NUM 100000

int main()
{
    char longstring[BIG_NUM];
    strcpy(longstring, "i'm actually pretty small");
    lprintf("%s", longstring);
    return 0;
}