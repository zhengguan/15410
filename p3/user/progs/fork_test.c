#include <simics.h>
#include <syscall.h>

int main() {

  int oldtid = gettid();
  lprintf("initial hello from %d", oldtid);

  int newtid;
  MAGIC_BREAK;
  if ( (newtid = fork()) ) {
    lprintf("parent: %d\t\tchild:%d", gettid(), newtid);
    while(1);

  } else {
    lprintf("child: %d", gettid());
    while(1);
  }

  return 0;

}