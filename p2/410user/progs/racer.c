/**
 * @file racer.c
 *
 * @brief a very strenuous test of mutexes, condition variables, and semaphores
 *
 * @author Peter Nelson <pnelson>
 *
 * This creates <threads> number of threads.  Each thread gets 1 line of the
 * terminal.  Every thread attempts to aquire the semaphore which was
 * initialized with a value of <semaphores>.  When a thread aquires the sem
 * it prints out <runlength> number of characters, one at a time.
 *
 * There is an additional <pausetime> that can be used to cause each thread
 * to call sleep of that value.  Finally there is a "bad" thread that
 * cycles misbehave states and then sleeps for <misbehavetime>.
 * 
 * The expected default result is seeing a group of 12 threads run out for 5
 * characters.  Then the next group of 12 threades will run out for 5 characters.
 * The bottom-right corner shows the current number of threads that are active
 * (should always be <=12) and the current misbehave state.
 * 
 * threads and semaphores should be set to 24 and 12 for best effect
 * runlength should be set to 5-10 to be able to see how many are active,
 * or set to 1 for the most strenuous
 *
 * Colors are pseudo-randomly rotated by continuously adding get_ticks() to
 * the color value.  No real point other than to be trippy.
 * 
 * Arguments: program threads runlength semaphores pausetime misbehavetime
 */

#include <thread.h>
#include <syscall.h>
#include <simics.h>
#include <stdio.h>
#include <sem.h>
#include <mutex.h>
#include <stdlib.h>

#define STACK_SIZE 4096
#define MAX_MISBEHAVE 64

// Uses my static initializer, you might have to change this
mutex_t lock_print;

// Default values
int threads = 24;
int run = 5;
int sems = 12;
sem_t sem;
int pause = 0;
int bad_sleep = 50;

char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
char empty[81];
mutex_t lock_running;
int running = 0;

char color = 0;

#define expect(exp) \
    { \
        err = exp; \
        if(err < 0) \
            lprintf("%s:%u: returned %d", \
                    __FILE__, __LINE__, err); \
    }

void *race(void *arg)
{
    int err;
    int col = -1;
    int i;
    int row = (int)arg;
    
    mutex_lock(&lock_print);
    color += get_ticks();
    set_term_color(color);
    expect(set_cursor_pos(row, 0));
    printf("%s", &empty[0]);
    mutex_unlock(&lock_print);

    while(1)
    {
        sem_wait(&sem);

        mutex_lock(&lock_running);
        int r = ++running;
        mutex_unlock(&lock_running);
        if(r > sems)
            lprintf("BAD! I have %d > %d threads running", r, sems);
        
        mutex_lock(&lock_print);
        expect(set_cursor_pos(24, 77));
        printf("%.2d", running);
        mutex_unlock(&lock_print);
        
        for(i = 0; i < run; i++)
        {
            col++;
            if(80 == col)
            {
                mutex_lock(&lock_print);
                color += get_ticks();
                set_term_color(color);
                expect(set_cursor_pos(row, 0));
                printf("%s", &empty[0]);
                mutex_unlock(&lock_print);
                col = 0;
            }
            
            mutex_lock(&lock_print);
            color += get_ticks();
            set_term_color(color);
            expect(set_cursor_pos(row, col));
            printf("%c", alphabet[i]);
            mutex_unlock(&lock_print);

            if(pause)
                expect(sleep(pause));
            thr_yield(-1);
        }

        mutex_lock(&lock_running);
        running--;
        mutex_unlock(&lock_running);
        
        sem_signal(&sem);
    }
}

void *bad(void *arg)
{
    while(1)
    {
        int err;
        int i = (int)arg;

        for(i=-1; i<MAX_MISBEHAVE; i++)
        {
            lprintf("calling misbehave(%d)", i);
        
            mutex_lock(&lock_print);
            expect(set_cursor_pos(24, 73));
            printf("% .2d", i);
            mutex_unlock(&lock_print);

            misbehave(i);
            sleep(bad_sleep);
        }
    }
    return arg;
}

int main( int argc, char *argv[] )
{
    int i, err;

    switch(argc) {
        case 6:
            i = atoi(argv[5]);
            if(i >= 0)
                bad_sleep = i;
        case 5:
            i = atoi(argv[4]);
            if(i >= 0)
                pause = i;
        case 4:
            i = atoi(argv[3]);
            if(i > 0)
                sems = i;
        case 3:
            i = atoi(argv[2]);
            if(i <= 26 && i > 0)
                run = i;
        case 2:
            i = atoi(argv[1]);
            if(i > 1 || i <= 24)
                threads = i;
        default:
            break;
    }

    expect(thr_init(STACK_SIZE));
    expect(mutex_init(&lock_print));
    expect(mutex_init(&lock_running));
    expect(sem_init(&sem, sems));

    for(i=0; i<80; i++)
        empty[i] = ' ';
    empty[80] = '\0';
   
    expect(thr_create(bad, (void*)i) >= 0);
    
    for(i=0; i<threads; i++)
        expect(thr_create(race, (void*)i) >= 0);
    
    getchar();
    expect(set_cursor_pos(24,0));
    
    task_vanish(0);
}
