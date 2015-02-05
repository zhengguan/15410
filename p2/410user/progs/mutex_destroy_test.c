/**
 * @file mutex_destroy_test.c
 * @brief A test of mutex_destroy()
 *
 *
 * @author Joey Echeverria (jge)
 * @bug No known bugs.
 */

#include <thread.h>
#include <mutex.h>
#include <syscall.h>
#include <stdio.h>
#include "410_tests.h"
DEF_TEST_NAME("mutex_destroy_test:");

#define STACK_SIZE 4096

mutex_t mutex;

/**
 * @brief
 *
 * @param argc The number of arguments
 * @param argv The argument array
 */
int main(int argc, char *argv[])
{
	int error;
  REPORT_LOCAL_INIT;

  REPORT_START_ABORT;
	if((error = thr_init(STACK_SIZE)) < 0) {
		REPORT_ERR("thr_init() returned error %d", error);
		REPORT_END_FAIL;
    return -10;
	}

	if((error = mutex_init(&mutex)) < 0) {
		REPORT_ERR("mutex_init() returned error %d", error);
		REPORT_END_FAIL;
		return -20;
	}

	mutex_lock(&mutex);

	mutex_destroy(&mutex);

	REPORT_END_FAIL;
	thr_exit((void *)1);
	return 1;
}
