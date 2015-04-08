#include <simics.h>
#include <syscall.h>
#include <thread.h>

void *worker(void *null)
{
    set_status(42);
    lprintf("worker exiting");
    return 0;
}

int main()
{


    int oldtid = gettid();
    lprintf("initial hello from %d", oldtid);

    int newtid1, newtid2;
    if ( (newtid1 = fork()) ) {
        if ( (newtid2 = fork()) ) {
            int status1;
            lprintf("waiting");
            wait(&status1);
            lprintf("Child exited with status %d", status1);
            //wait(&status2);
            //lprintf("Child exited with status %d", status2);
            lprintf("parent exiting");
            return 0;
        }
        lprintf("child2");
        sleep(1000);
        return gettid();
    }

    lprintf("child1");
    sleep(3000);
    return gettid();
}