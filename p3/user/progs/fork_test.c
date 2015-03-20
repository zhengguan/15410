#include <simics.h>
#include <syscall.h>

int main() {

  int oldtid = gettid();
  lprintf("initial hello from %d", oldtid);

  int newtid;

  if ( (newtid = fork()) ) {
    lprintf("hello from %d", newtid);
    while(1);

  } else {
    lprintf("hello from %d", oldtid);
  }

  return 0;

}