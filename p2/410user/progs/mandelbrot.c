/*!
 * @file mandelbrot.c
 *
 * @brief A very cool, fractal test.
 *
 * @author Keith Bare
 */

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <syscall.h>
#include <simics.h>
#include "rand.h"

#include "thread.h"
#include "cond.h"
#include "mutex.h"
#include "thrgrp.h"

/* Modified code copied from shell.c */
/* Since P3 does not require GETCHAR, we define it here */
#ifndef HAVE_GETCHAR
#define getchar static_getchar
static char getchar(void)
{
  char buf[2];
  readline(2, buf);

  return buf[0];
}
#endif

#define CONSOLE_WIDTH  80  /*!<@brief Width of the console */
#define CONSOLE_HEIGHT 25  /*!<@brief Height of the console */

#define FRACTIONAL_BITS 24 /*!<@brief Number of fixed point fractional bits */

#define FIXEDPOINT_TWO  0x02000000 /*!<@brief Fixed point representation of
                                        two */
#define FIXEDPOINT_FOUR 0x04000000 /*!<@brief Fixed point representation of
                                        four */

#define YIELD_FACTOR 8 /*!<@brief Higher => thread is less likely to yield */

#define MAX_TRAPS 30   /*!<@brief Maximum number of support traps */

/*!
 * @brief Look-up table for finding the fixed-point real coordinate
 * for a column.
 */
const int col_table[] = {
    0xfd800000,
    0xfd8ccccc,
    0xfd999999,
    0xfda66666,
    0xfdb33333,
    0xfdc00000,
    0xfdcccccc,
    0xfdd99999,
    0xfde66666,
    0xfdf33333,
    0xfe000000,
    0xfe0ccccc,
    0xfe199999,
    0xfe266666,
    0xfe333333,
    0xfe400000,
    0xfe4ccccc,
    0xfe599999,
    0xfe666666,
    0xfe733333,
    0xfe800000,
    0xfe8ccccc,
    0xfe999999,
    0xfea66666,
    0xfeb33333,
    0xfec00000,
    0xfecccccc,
    0xfed99999,
    0xfee66666,
    0xfef33333,
    0xff000000,
    0xff0ccccc,
    0xff199999,
    0xff266666,
    0xff333333,
    0xff400000,
    0xff4ccccc,
    0xff599999,
    0xff666666,
    0xff733333,
    0xff800000,
    0xff8ccccc,
    0xff999999,
    0xffa66666,
    0xffb33333,
    0xffc00000,
    0xffcccccc,
    0xffd99999,
    0xffe66666,
    0xfff33333,
    0x00000000,
    0x000ccccc,
    0x00199999,
    0x00266666,
    0x00333333,
    0x00400000,
    0x004ccccc,
    0x00599999,
    0x00666666,
    0x00733333,
    0x00800000,
    0x008ccccc,
    0x00999999,
    0x00a66666,
    0x00b33333,
    0x00c00000,
    0x00cccccc,
    0x00d99999,
    0x00e66666,
    0x00f33333,
    0x01000000,
    0x010ccccc,
    0x01199999,
    0x01266666,
    0x01333333,
    0x01400000,
    0x014ccccc,
    0x01599999,
    0x01666666,
    0x01733333};

/*!
 * @brief Look-up table for finding the fixed-point imaginary
 * coordinate for a row.
 */
const int row_table[] = {
    0xfe800000,
    0xfea00000,
    0xfec00000,
    0xfee00000,
    0xff000000,
    0xff200000,
    0xff400000,
    0xff600000,
    0xff800000,
    0xffa00000,
    0xffc00000,
    0xffe00000,
    0x00000000,
    0x00200000,
    0x00400000,
    0x00600000,
    0x00800000,
    0x00a00000,
    0x00c00000,
    0x00e00000,
    0x01000000,
    0x01200000,
    0x01400000,
    0x01600000};

/*!
 * @brief Type for state that the threads modify
 */
typedef struct mandelbrot_state {
    mutex_t cell_mutex; /*!< Mutex protecting this cell */
    int count;          /*!< Number of threads that have visited this cell */
    int color;
    int trap_num;       /*!< -1, or the index of the condition variable in
                             which visited threads are trapped */
    int trapped_threads;/*!< Number of threads trapped at this cell */
    int exiting;        /*!< Indicates that the program is terminating */
} mandelbrot_state_t;


mutex_t console_lock;    /*!<@brief Mutex protecting the console */

cond_t traps[MAX_TRAPS]; /*!<@brief Array of condition variables in which
                              threads can get &quot;stuck&quot; */

int exiting = 0;         /*!<@brief Flag the indicates the test is exiting */

int wakeup_threshold = 1;/*!<@brief Maximum number of waiting threads */

/*!
 * @brief Shared state that the threads modify.
 */
mandelbrot_state_t state[CONSOLE_HEIGHT - 1][CONSOLE_WIDTH];


/*!
 * @brief Fixed point multiplication.
 *
 * This is so I can do cool things, even though Pebbles doesn't allow programs
 * to use the FPU.
 */
int fixed_mult(int a, int b)
{
    return (int)((((long long)a) * ((long long)b)) >> FRACTIONAL_BITS);
}

/*!
 * @brief Calculates the color that a (row, column) pair of the
 * mandelbrot set should be.
 *
 * Returns the number of iterations of the Mandelbrot equation, Z =
 * (Z_{n - 1})^2 + Z_0, occur before Z is more than a distance of one
 * from the complex origin.
 */
int mandelbrot_calc(int row, int col)
{
    int i;

    int imag_coord_init = row_table[row];
    int real_coord_init = col_table[col];
    int imag_coord = imag_coord_init;
    int real_coord = real_coord_init;

    for (i = 0;
         i < 14 && fixed_mult(real_coord, real_coord) + fixed_mult(imag_coord, imag_coord) < FIXEDPOINT_FOUR;
         i++) {

        int tmp_imag_coord = fixed_mult(FIXEDPOINT_TWO,
                                        fixed_mult(real_coord, imag_coord)) +
                             imag_coord_init;
        real_coord = fixed_mult(real_coord, real_coord) -
                     fixed_mult(imag_coord, imag_coord) +
                     real_coord_init;
        imag_coord = tmp_imag_coord;
    }

    return i;
}

/*!
 * @brief Processes and draws a &quot;pixel&quot;.
 *
 * Assumes that the caller has console_lock.
 *
 * @param row row of the pixel to draw
 * @param col column of the pixel to draw
 */
void process_pixel(int row, int col)
{
    mutex_lock(&state[row][col].cell_mutex);

    if (state[row][col].exiting) {
        mutex_unlock(&state[row][col].cell_mutex);
        return;
    }

    if (state[row][col].count < 0) {
        state[row][col].color = mandelbrot_calc(row, col);
    }

    if (state[row][col].trapped_threads >= wakeup_threshold) {
        cond_broadcast(&traps[state[row][col].trap_num]);
    } else if (state[row][col].trap_num >= 0) {
        state[row][col].trapped_threads++;

        mutex_lock(&console_lock);
        set_cursor_pos(row, col);
        set_term_color((15 - state[row][col].color) | BGND_RED);
        printf("%c", '0' + state[row][col].trapped_threads % 10);
        mutex_unlock(&console_lock);

        cond_wait(&traps[state[row][col].trap_num],
                  &state[row][col].cell_mutex);

        state[row][col].trapped_threads--;
    }

    state[row][col].count++;

    mutex_lock(&console_lock);
    set_cursor_pos(row, col);
    set_term_color((15 - state[row][col].color) | BGND_BLACK);
    printf("%c", 'a' + state[row][col].count % 26);
    mutex_unlock(&console_lock);

    mutex_unlock(&state[row][col].cell_mutex);
}

/*!
 * @brief Main function for wanderer_threads.
 *
 * @param arg
 */
void *wanderer_main(void *arg)
{
    int row, col;

    row = genrand() % (CONSOLE_HEIGHT - 1);
    col = genrand() % CONSOLE_WIDTH;

    while (!exiting) {

        process_pixel(row, col);

        row = genrand() % (CONSOLE_HEIGHT - 1);
        col = genrand() % CONSOLE_WIDTH;

        if (genrand() % YIELD_FACTOR == 0) {
            yield(-1);
        }

        mutex_lock(&console_lock);
        set_cursor_pos(row, col);
        set_term_color(FGND_BLACK | BGND_BLUE);
        printf("%c", '+');
        mutex_unlock(&console_lock);

        if (genrand() % YIELD_FACTOR == 0) {
            yield(-1);
        }

    }

    return NULL;
}

/*!
 * @brief Prints a usage message.
 */
void usage() {
    printf("mandelbrot [num_threads] [num_traps] [wakeup_threshold]\n\n");
    printf("num_traps must be <= %d\n", MAX_TRAPS);
    printf("if num_traps * (wakeup_threshold - 1) >= num_threads "
           "deadlock may result\n");
}

/*!
 * @brief Code entry-point.
 *
 * @param argc
 * @param argv
 */
int main(int argc, char *argv[])
{
    thrgrp_group_t thread_group;
    int num_threads = 15;
    int num_traps = 8;
    void *status;

    int i;
    int row, col;
    char c;

    if (argc >= 2) {
        if (sscanf(argv[1], "%d", &num_threads) < 1 || num_threads <= 0) {
            usage();
            return 0;
        }
    }
    if (argc >= 3) {
        if (sscanf(argv[2], "%d", &num_traps) < 1 || num_traps < 0 ||
            num_traps > MAX_TRAPS) {
            usage();
            return 0;
        }
    }
    if (argc >= 4) {
        if (sscanf(argv[3], "%d", &wakeup_threshold) < 1) {
            usage();
            return 0;
        }
    }

    sgenrand(get_ticks() | 15410); /* sgenrand(0) is illegal, so... */

    lprintf("Initializing thread library...\n");
    thr_init(PAGE_SIZE * 10);

    lprintf("Initializing console lock...\n");
    assert(mutex_init(&console_lock) >= 0);

    lprintf("Initializing state...\n");
    for (row = 0; row < CONSOLE_HEIGHT - 1; row++) {
        for (col = 0; col < CONSOLE_WIDTH; col++) {
            assert(mutex_init(&state[row][col].cell_mutex) >= 0);
            state[row][col].count = -1;
            state[row][col].trap_num = -1;
            state[row][col].trapped_threads = 0;
            state[row][col].exiting = 0;
        }
    }

    lprintf("Initializing traps...\n");
    /* May not assign all the traps, but it doesn't matter. */
    for (i = 0; i < num_traps; i++) {
        assert(cond_init(&traps[i]) >= 0);
        state[genrand() % (CONSOLE_HEIGHT - 1)][genrand() % CONSOLE_WIDTH].trap_num = i;
    }

    lprintf("Clearing console...\n");
    for (i = 0; i < CONSOLE_HEIGHT; i++) {
        printf("\n");
    }

    lprintf("Initializing thread group...\n");
    assert(thrgrp_init_group(&thread_group) >= 0);

    lprintf("Creating wanderer threads...\n");
    for (i = 0; i < num_threads; i++) {
        thrgrp_create(&thread_group, &wanderer_main, NULL);
    }

    lprintf("Waiting to exit...\n");
    while ((c = getchar()) != 'q') {
        lprintf("got %c!\n", c);
        continue;
    }

    exiting = 1;

    for (row = 0; row < CONSOLE_HEIGHT - 1; row++) {
        for (col = 0; col < CONSOLE_WIDTH; col++) {
            mutex_lock(&state[row][col].cell_mutex);
            state[row][col].exiting = 1;
            mutex_unlock(&state[row][col].cell_mutex);
        }
    }

    for (i = 0; i < num_traps; i++) {
        cond_broadcast(&traps[i]);
    }

    lprintf("Joining with wanderer threads...\n");
    for (i = 0; i < num_threads; i++) {
        thrgrp_join(&thread_group, &status);
    }

    lprintf("Destroying thread group...\n");
    thrgrp_destroy_group(&thread_group);

    for (row = 0; row < CONSOLE_HEIGHT - 1; row++) {
        for (col = 0; col < CONSOLE_WIDTH; col++) {
            mutex_destroy(&state[row][col].cell_mutex);
        }
    }

    for (i = 0; i < num_traps; i++) {
        cond_destroy(&traps[i]);
    }

    mutex_destroy(&console_lock);

    set_cursor_pos(CONSOLE_HEIGHT - 1, 0);
    set_term_color(FGND_CYAN | BGND_BLACK);

    thr_exit(NULL);
    return 0;
}
