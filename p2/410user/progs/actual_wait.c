/** @file 410user/progs/actual_wait.c
 *  @author de0u
 *  @brief Exercises wait() and vanish().
 *  @public yes
 *  @for p2 p3
 *  @covers wait vanish fork yield set_status
 *  @status done
 */

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

#include "410_tests.h"
DEF_TEST_NAME("actual_wait:");

#define NCHILD 40 /* must be < 255 */
#define PRESERVE (1024*1024)

void child(int);

int
main(int argc, char *argv[])
{
  char buf[4096];
  int pid, pids[NCHILD];
  int c, preserve;
  char msg[128];

  buf[0] = buf[sizeof (buf) - 1] = 'q';

  REPORT_START_CMPLT;

  for (c = 0; c < NCHILD; ++c)
    switch(pid = fork()) {
    case -1:
      REPORT_MISC("cannot fork");
	  goto fail;
    case 0:
      child(c);
      break;
    default:
      pids[c] = pid;

      snprintf(msg, sizeof (msg), "slot %d pid %d", c, pid);
      printf("%s\n", msg);
      REPORT_MISC(msg);

      if (c & 1)
        yield(-1);
      break;
    }

  for (c = 0; c < NCHILD; ++c) {
    int slot;
    char msg[128];

    if ((c & 8) == 8)
      yield(-1);

    pid = wait(&slot);

    snprintf(msg, sizeof (msg), "slot %d pid %d", slot, pid);
    printf("%s\n", msg);
    REPORT_MISC(msg);

    if ((slot < 0) || (slot >= NCHILD)) {
      REPORT_MISC("invalid slot");
	  goto fail;
    }
    if (pids[slot] != pid) {
      REPORT_MISC("pid/slot mismatch");
	  goto fail;
    }
    pids[slot] = -1;
  }

  preserve = PRESERVE;
  pid = wait(&preserve);
  if (pid >= 0) {
    REPORT_MISC("phantom menace");
    goto fail;
  }
  if (preserve != PRESERVE) {
    REPORT_MISC("obliteration");
    goto fail;
  }

  REPORT_END_SUCCESS;
  exit(0);

fail:
  REPORT_END_FAIL;
  exit(9);
}

void
child(int which)
{
  int wpid, gpid;
  char msg[128];

  snprintf(msg, sizeof (msg), "child %d", which);
  printf("%s\n", msg);
  REPORT_MISC(msg);

  switch (gpid = fork()) {
  case -1:
    REPORT_MISC("child cannot fork");
    goto fail;
    break;
  case 0:
    if (which & 1) {
      yield(-1);
      yield(-1);
    }
    exit(0);
  default:
    if ((which & 4) != 4) {
      if ((wpid = wait(0)) != gpid) {
        REPORT_MISC("grandchild??");
        goto fail;
      }
    }
    exit(which);
    break;
  }

fail:
  exit(-1);
}
