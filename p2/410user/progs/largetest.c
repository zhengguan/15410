/**
 * @file largetest.c
 *
 * **> Public: yes
 * **> Covers: thr_create,thr_join
 * **> For: P2
 * **> NeedsWork: No
 * **> Authors: tchitten
 *
 * @brief creates succesfully larger waves of threads until something bad 
 * happens. 
 *
 * This test continually tries to create larger and larger thread waves until
 * something bad happens at which point it exits. The purpose of this test is
 * to determine if that "bad thing" is the "expected thing."  On some 
 * implementations it has been observed to reach approximately 7000 threads
 * (round 13) before thr_create fails.
 * 
 * Each round works by spawning a single thread, which goes on to spawn two
 * threads, each of whom spawn two more threads until 2^n-1 threads have been
 * spawned.  Each of these threads does a bit of work and puts its tid at a
 * pseudo random index in the free_order table.  This index is determined by
 * running I rounds of an N-bit LFSR where I is the creation index of the 
 * thread, and N is the round number (corresponding to the 2^n-1 thread). 
 * Using a proper feedback term, each iteration of the LFSR will produce a
 * unique index between 1 and 2^N-1 for the first 2^N-1 iterations, at which 
 * point it will wrap.  Thanks to Professor Koopman for hosting the LFSR terms! 
 *
 * Once the threads have been spawned and have put their TIDs in their 
 * pseudo random index, the main thread then proceeds to join on each thread
 * in this random order.  If at any point a thread cannot be created it sets
 * the error flag and all threads stop.  The main thread then proceeds to join
 * on all the threads in spawn order.
 *
 * @author Tom Chittenden (tchitten), Fall 2014
 *
 * @bug None known
 */

#include <syscall.h>
#include <simics.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread.h>
#include <thrgrp.h>
#include <assert.h>
#include "410_tests.h"
DEF_TEST_NAME("largetest:");

#define SLEEP_SOME 0x00
#define YIELD_SOME 0x01

// http://users.ece.cmu.edu/~koopman/lfsr/index.html
#define LFSR_MINTERM 1
#define LFSR_MAXTERM (LFSR_MINTERM + (sizeof(terms)/sizeof(*terms)) - 1)
uint16_t terms[] = {
    0x1, // 1
    0x3, // 2
    0x6, // 3
    0xC, // ...
    0x17,
    0x33,
    0x69,
    0x8E,
    0x1F4,
    0x2E3,
    0x4C7,
    0xA1B,
    0x10F1,
    0x20CE,
    0x41D7,
    0x8233  // 16
};

int error = 0;
int round = 0;
int lfsr_seed = 1;
int* free_order;
int* spawn_order;

int pow2(int n) {
    return 1 << n;
}

/*
 * ispow2() can be done faster by applying
 * 15-213 "data lab" optimizations, but we
 * decline.
 */
int ispow2(int n) {
    int b, count;

    for (b = 0, count = 0; b < 32; ++b)
        if (((unsigned int) n) & (1 << b))
            ++count;

    return (count < 2);
}

/*
 * next_pow2() can be done faster by applying
 * 15-213 "data lab" optimizations, but we
 * decline.
 */
int next_pow2(int n) {
    int ret = 1;

    while (ret <= n)
        ret *= 2;
    return ret;
}

/*
 *  @brief computes n iterations of an LFSR using the given feedback and seed.
 */
int lfsr(int round, int seed, int n) {
    assert(LFSR_MINTERM <= round && round <= LFSR_MAXTERM);
    assert(seed != 0 && seed <= pow2(round));
    
    int i, cur = seed, feedback = terms[round - LFSR_MINTERM];
    for(i = 0; i < n; i++) {
        if(cur & 1) { cur = (cur >> 1) ^ feedback; }
        else        { cur = (cur >> 1); }   
    }
    
    assert(cur != 0);
    return cur;
}

void* thr(void *arg) {
    
    int i, idx = (int)arg;

    // Get the next power of 2.
    int bidx = next_pow2(idx);

    // We haven't generated all the threads yet, create some more!
    if (!error && bidx < pow2(round)) {
        // Get our relative index from the current base.
        int ridx = idx - bidx/2;
        
        // Get the indices of our children.
        int idx1 = bidx + 2*ridx;
        int idx2 = bidx + 2*ridx + 1;

        // Try to create the first thread. Set error on fail.
        int res1 = thr_create(thr, (void*)idx1);
        if(res1 < 0) {
            error = 1;
            goto ret;
        } else {
            spawn_order[idx1 - 1] = res1;
        }

        // Try to create the second thread. Set error on fail.
        int res2 = thr_create(thr, (void*)idx2);
        if(res2 < 0) {
            error = 1;
            goto ret;
        } else {
            spawn_order[idx2 - 1] = res2;
        }
    }

    // Place ourselves in the free_order array.
    int die_idx = lfsr(round, lfsr_seed, idx);
    free_order[die_idx - 1] = thr_getid();

    // And do some work.
    switch (idx % 2) {
        case SLEEP_SOME:
            sleep(idx % 100);
            break;
        case YIELD_SOME:
            for(i = 0; i < 5; i++) {
                yield(-1);
            }
            break;
    }

ret:
    return (void*)idx;
}


int main() {
    int i, res;

    // Given 32K stack sizes, it should be impossible to generate 2^16 threads
    // on a machine with 256 MB memory.
    thr_init(32*1024);
    REPORT_START_ABORT;

    // For each term in our LFSR feedback table, spawn 2^n-1 threads.
    for (round = LFSR_MINTERM; !error && round <= LFSR_MAXTERM; round++) {
        int nthreads = pow2(round) - 1;

        free_order = calloc(nthreads, sizeof(int));
        spawn_order = calloc(nthreads, sizeof(int));
        if(free_order == NULL || spawn_order == NULL) {
            lprintf("ROUND %d: error allocating %d byte buffers\n", 
                round, nthreads*sizeof(int));
        }

        // Spawn the first thread.
        res = thr_create(thr, (void*)1);
        if (res < 0) {
            lprintf("ROUND %d: failed before spawning 1 thread\n", round);
            break;
        } else {
            spawn_order[0] = res;
        }

        // Iterate over the randomly ordered threads.
        int successes = 0;

        for (i = 0; i < nthreads; i++) {
            while (free_order[i] == 0) {
                if (error) {
                    // Someone has encountered an error condition, break out 
                    // and join on the threads sequentially.
                    break;
                }

                // 'i'th element has not been populated, yield and try again.
                // This is not "good practice", but this is test code.
                yield(-1);
            }
            if (error) {
                // There's been an error. Break out.
                break;
            }

            // Join on the thread and clear it in the spawn_order array. This 
            // prevents us from joining on a thread twice in case there's an error.
            int thr_idx;

            res = thr_join(free_order[i], (void*)&thr_idx);
            if ( res < 0 ) {
                // There was an error joining on the thread! Bail out and
                // finish by joining in creation order.
                lprintf("ROUND %d: failed to join on thread %d!\n", 
                            round, free_order[i]);
                error = 1;
            } else {
                // Succesfully joined. Clear this thread from the spawn array 
                // to prevent joining on it twice in case there's an error.
                spawn_order[thr_idx - 1] = 0;
                successes++;
            }
        }

        if (error) {
            // Join on every thread we've created. We know if thread 'i' has 
            // finished then thread 'i+1' has an entry in the spawn_order 
            // array (or failed to be created)
            for (i = 0; i < nthreads; i++) {
                if(spawn_order[i] != 0) {
                    thr_join(spawn_order[i], NULL);
                    successes++;
                }
            }
            lprintf("ROUND %d: failed after spawning %d threads\n", 
                        round, successes);
            break;

        } else {
            lprintf("ROUND %d: succesfully spawned/reaped %d threads\n", 
                        round, successes);
            assert (nthreads == successes);
        }

        // Generate some new pseudorandom seed and free the old arrays. 
        lfsr_seed = lfsr(round, lfsr_seed, round);
        free(free_order);
        free(spawn_order);
    }

    if (error) {
        REPORT_END_FAIL;
    } else {
        // You managed to find 2 GB of thread stacks out of 256 MB of memory, 
        // good job.
        REPORT_END_SUCCESS;
    }
    thr_exit(0);
    return 0;
}
