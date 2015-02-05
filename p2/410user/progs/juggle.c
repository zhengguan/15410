/** @file juggle.c
 *  @brief A tester in which threads juggle thread juggle threads.
 *
 *     USAGE: juggle <num_levels> <num_throws> <rep> <misbehave_num + 1>
 *
 *  @author Cornell Wright (cgwright)
 *  @bug None known
 */
#include <stdio.h>
#include <rand.h>
#include <syscall.h>
#include <simics.h>
#include <thread.h>
#include <stdlib.h>
#include <mutex.h>

/** @brief Max amount of time a "ball" can "hang" in the air */
#define SLEEP_MAX 5

/** @brief Define this to print the level number each time a "ball" is caught
 */
#define PRINT

/** @brief Define this to count the number of threads */
#define COUNT_THREADS

/** @brief The number of throws to make in each level */
int n_throws;

#ifdef COUNT_THREADS

/** @brief What level to print the number of threads that have been created
 */
#define PRINT_LEVEL 5

/** @brief How many threads we have created since we started */
int th_count = 0;

/** @brief Protects counting */
mutex_t count_mutex;


/** @brief Increments the th_count in a thread safe manner. */
void inc_count(void)
{
    mutex_lock(&count_mutex);
    th_count++;
    mutex_unlock(&count_mutex);
    return;
}

/** @brief Prints count if n >= PRINT_LEVEL 
 *
 *  @param n what level we are calling form.
 */
void print_count(int n)
{
    int my_count;

    if (n >= PRINT_LEVEL) {
        return;
    }

    mutex_lock(&count_mutex);
	my_count = th_count;
    mutex_unlock(&count_mutex);
    lprintf("Thread count = %d\n", my_count);
    return;
}

#else

#define inc_count() ;
#define print_count(n) ;

#endif /* COUNT_THREADS */


/** @brief The workhorse thread of the test.
 *
 *  This thread recursively spawns two copies of itself decrementing n_voidstar
 *  so long as n_voidstar is positive. Each thread repeats this process n_throws
 *  times, after joining on the threads it created.
 *
 *  @param The level we are at (to keep us from infinitly recursively spawning.
 */
void *juggle(void * n_voidstar)
{
    int sub1, sub2;
    int throws;
    int substat;
    int ret;
    int n = (int)n_voidstar;

    inc_count();
    print_count(n);

    if (n > 0) {
        for (throws = 0; throws < n_throws; throws++) {
    
            // Toss up two balls
            sub1 = thr_create(juggle, (void *)(n - 1));
        
            if (sub1 < 0) {
                lprintf("Lev %d failed to create first thread w/ err %d\n",
                        n, sub1);
            }
    
            sub2 = thr_create(juggle, (void *)(n - 1));
    
            if (sub2 < 0) {
                lprintf("Lev %d failed to create second thread w/ err %d\n",
                        n, sub2);
            }
    
            // Try to catch them
            if ((ret = thr_join(sub1, (void*)&substat))
                 != 0 || substat != (n - 1)) {
              lprintf("Lev %d failed to join first thread correctly:\n\t", n);
              lprintf("join(%d), ret = %d, %d ?= %d\n",
                                    sub1, ret, (n - 1), substat);
            }
            
            if ((ret = thr_join(sub2, (void*)&substat))
                != 0 || substat != (n - 1)) {
              lprintf("Lev %d failed to join second thread correctly:\n\t", n);
              lprintf("join(%d), ret = %d, %d ?= %d\n",
                                    sub2, ret, (n - 1), substat);
            }
        }
    }
#ifdef PRINT
    // Tell that we were successfull.
    putchar((char)n + '0');
#endif

    print_count(n);

    // Hang in the air for some amount of time
    sleep(genrand() % SLEEP_MAX);
    
    return (void *)n;
}

/** @brief Prints the usage parameters.
 *
 *  @param What our executable is called.
 */
void print_usage(char *prog_name)
{
    printf("USAGE: %s <num_levels> <num_throws> <rep> <misbehave_num + 1>\n",
           prog_name);
}

int main(int argc, char **argv)
{
    int n_levels;
    int ret;
    int i, rep;
    int misbehave_num;

    if (argc != 5) {
        printf("Wrong number of args.\n\n");
        print_usage(argv[0]);
        return -1;
    }

    n_levels = atoi(argv[1]);
    n_throws = atoi(argv[2]);
    rep = atoi(argv[3]);
    misbehave_num = atoi(argv[4]) - 1;

    if (n_levels < 0) {
        printf("Levels must be non-negative.\n\n");
        print_usage(argv[0]);
        return -1;
    }

    if (n_throws < 0) {
        printf("Throws must be non-negative.\n\n");
        print_usage(argv[0]);
        return -1;
    }

    if (thr_init(4096) != 0) {
        printf("Init failed. Something's busted.\n\n");
        return -10;
    }

    printf("MISBEHAVE: %d\n\n", misbehave_num);
    misbehave(misbehave_num);

    if (mutex_init(&count_mutex) != 0) {
        printf("Mutex init of count_mutex failed. Go fix your mutexes.\n\n");
        return -20;
    }

    
    for (i = 0; rep == 0 || i < rep; i++) {
        printf("Here we go! Repetition %d\n", i+1);

        if ((ret = (int)juggle((void *)n_levels)) != n_levels) {
     printf("Root juggle thread returned wrong value: %d should've been: %d\n",
                      ret, n_levels);
            return -2;
        }
        printf("\n\nSuccess. All balls accounted for.\n");

#ifdef COUNT_THREADS
        lprintf("Created and destroyed %d threads so far.\n", th_count);
#endif
    }

    thr_exit(0);
    printf("This is NOT happening!\n");
    return -30;
}
