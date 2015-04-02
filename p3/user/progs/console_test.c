#include <simics.h>
#include <syscall.h>

#define BUF_SIZE 80

int main() {
    
    char buf[BUF_SIZE];
    
    lprintf("Starting readline");
    MAGIC_BREAK;
    
    readline(BUF_SIZE, buf);
  
    while(1);
  
    return 0;

}
